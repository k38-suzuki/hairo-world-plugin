/**
    SimpleUAV typeTv2 Controller
    @author Kenta Suzuki
*/

#include <cnoid/EigenUtil>
#include <cnoid/Joystick>
#include <cnoid/Rotor>
#include <cnoid/SimpleController>
#include <cnoid/SimplePilot>

using namespace cnoid;

namespace {

const double pgain[] = { 0.10, 0.003, 0.003, 0.00002 };
const double dgain[] = { 0.05, 0.002, 0.002, 0.00001 };

} // namespace

class UAVtypeTv2JoystickController : public SimpleController
{
    SimplePilot pilot;
    DeviceList<Rotor> rotors;
    Vector4 zref;
    Vector4 dzref, dzold;
    Vector2 dxref;
    double timeStep;

    Joystick joystick;

public:
    virtual bool initialize(SimpleControllerIO* io) override
    {
        Body* body = io->body();
        rotors = body->devices();

        if(pilot.initialize(io)) {
            pilot.setStickMode(StickMode::MODE2);
            if(!pilot.findRateGyroSensor("GyroSensor")) {
                return false;
            }
        }

        for(auto& rotor : rotors) {
            io->enableInput(rotor);
        }

        zref = pilot.zrpy();
        dzref = dzold = Vector4::Zero();
        dxref = Vector2::Zero();

        timeStep = io->timeStep();

        return true;
    }

    virtual bool control() override
    {
        joystick.readCurrentState();
        pilot.readCurrentState();

        static const int controlID[] = {
            FlightControl::THROTTLE, FlightControl::AILERON, FlightControl::ELEVATOR, FlightControl::RUDDER
        };

        double pos[4];
        for(int i = 0; i < 4; i++) {
            pos[i] = joystick.getPosition(pilot.axisId(controlID[i]));
            if(fabs(pos[i]) < 0.2) {
                pos[i] = 0.0;
            }
        }

        Vector4 fz = Vector4::Zero();
        Vector4 z = pilot.zrpy();
        Vector4 dz = pilot.dzrpy();
        Vector4 ddz = (dz - dzold) / timeStep;

        Vector2 dx_local = pilot.dxy_local();
        Vector2 ddx_local = pilot.ddxy_local();
        double gc = pilot.gravityCompensation(4);

        static const double P = 1.0;
        static const double D = 1.0;

        for(int i = 0; i < 4; ++i) {
            if(i == 3) {
                dzref[i] = -2.1 * pos[i];
                fz[i] = (dzref[i] - dz[i]) * pgain[i] + (0.0 - ddz[i]) * dgain[i];
            } else {
                if(i == 0) {
                    zref[i] += (-0.5 * timeStep) * pos[i];
                } else {
                    int j = i - 1;
                    dxref[j] = -4.0 * pos[i];
                    zref[i] = P * (dxref[j] - dx_local[1 - j]) + D * (0.0 - ddx_local[1 - j]);
                }
                if(i == 1) {
                    zref[i] *= -1.0;
                }
                fz[i] = (zref[i] - z[i]) * pgain[i] + (0.0 - dz[i]) * dgain[i];
            }
        }
        dzold = dz;

        static const double TD[4][4] = {
            {1.0, -1.0, -1.0, -1.0},
            {1.0,  1.0, -1.0,  1.0},
            {1.0,  1.0,  1.0, -1.0},
            {1.0, -1.0,  1.0,  1.0}
        };
        static const double ATD[] = { -1.0, 1.0, -1.0, 1.0 };

        for(size_t i = 0; i < rotors.size(); i++) {
            Rotor* rotor = rotors[i];
            double ft = gc + TD[i][0] * fz[0] + TD[i][1] * fz[1] + TD[i][2] * fz[2] + TD[i][3] * fz[3];
            rotor->force() = ft;
            rotor->torque() = ATD[i] * ft;
            rotor->notifyStateChange();
        }

        return true;
    }
};

CNOID_IMPLEMENT_SIMPLE_CONTROLLER_FACTORY(UAVtypeTv2JoystickController)