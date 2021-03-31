/**
   HobbyDrone Controller
   @author Kenta Suzuki
*/

#include <cnoid/AccelerationSensor>
#include <cnoid/EigenUtil>
#include <cnoid/SimpleController>
#include <cnoid/SharedJoystick>
#include <cnoid/RateGyroSensor>
#include <cnoid/Rotor>

using namespace std;
using namespace cnoid;

class HobbyDroneJoystickController : public SimpleController
{
public:
    SharedJoystickPtr joystick;
    int targetMode;
    BodyPtr ioBody;
    DeviceList<Rotor> rotors;
    RateGyroSensor* gyroSensor;
    AccelerationSensor* accSensor;
    double dt;

    Vector4 zref;
    Vector4 zprev;
    Vector4 dzref;
    Vector4 dzprev;

    Vector2 xyref;
    Vector2 xyprev;
    Vector2 dxyref;
    Vector2 dxyprev;

    bool on;
    bool manualMode;
    bool mode1;
    bool wailly;

    virtual bool initialize(SimpleControllerIO* io) override
    {
        ioBody = io->body();
        dt = io->timeStep();
        rotors = io->body()->devices();
        gyroSensor = ioBody->findDevice<RateGyroSensor>("GyroSensor");
        accSensor = ioBody->findDevice<AccelerationSensor>("AccSensor");
        on = true;
        manualMode = false;
        mode1 = true;
        wailly = false;

        for(auto opt : io->options()){
            if(opt == "manual"){
                manualMode = true;
            }
            if(opt == "mode2"){
                mode1 = false;
            }
            if(opt == "wailly") {
                wailly = true;
                on = true;
            }
        }

        io->enableInput(ioBody->rootLink(), LINK_POSITION);
        io->enableInput(gyroSensor);
        io->enableInput(accSensor);

        for(size_t i = 0; i < rotors.size(); i++) {
            Rotor* rotor = rotors[i];
            io->enableInput(rotor);
        }

        zref = zprev = getZRPY();
        dzref = dzprev = Vector4::Zero();
        xyref = xyprev = getXY();
        dxyref = dxyprev = Vector2::Zero();

        joystick = io->getOrCreateSharedObject<SharedJoystick>("joystick");
        targetMode = joystick->addMode();

        return true;
    }

    virtual bool control() override
    {
        joystick->updateState(targetMode);

        static bool pprev = false;
        bool p = joystick->getButtonState(targetMode, Joystick::A_BUTTON);
        if(p && !pprev) {
            on = !on;
        }
        pprev = p;

        static bool mprev = false;
        bool m = joystick->getButtonState(targetMode, Joystick::B_BUTTON);
        if(m && !mprev) {
            mode1 = !mode1;
        }
        mprev = m;

        static const int mode1ID[] = {
            Joystick::R_STICK_V_AXIS,
            Joystick::R_STICK_H_AXIS,
            Joystick::L_STICK_V_AXIS,
            Joystick::L_STICK_H_AXIS
        };

        static const int mode2ID[] = {
            Joystick::L_STICK_V_AXIS,
            Joystick::R_STICK_H_AXIS,
            Joystick::R_STICK_V_AXIS,
            Joystick::L_STICK_H_AXIS
        };

        static const int waillyMode1ID[] = { 1, 0, 2, 5 };

        static const int waillyMode2ID[] = { 2, 0, 1, 5 };

        int axisID[4] = { 0 };
        if(mode1) {
            for(int i = 0; i < 4; i++) {
                if(!wailly) {
                    axisID[i] = mode1ID[i];
                } else {
                    axisID[i] = waillyMode1ID[i];
                }
            }
        } else {
            for(int i = 0; i < 4; i++) {
                if(!wailly) {
                    axisID[i] = mode2ID[i];
                } else {
                    axisID[i] = waillyMode2ID[i];
                }
            }
        }

        static const double P[] = { 10.0, 1.0, 1.0, 0.01 };
        static const double D[] = { 5.0, 1.0, 1.0, 0.002 };
        static const double X[] = { -0.001, 1.0, -1.0, -1.0 };

        static const double KP[] = { 1.0, 1.0 };
        static const double KD[] = { 1.0, 1.0 };
        static const double KX[] = { -1.0, -1.0 };

        static const double sign[4][4] = {
            { 1.0, -1.0, -1.0, -1.0 },
            { 1.0, 1.0, -1.0, 1.0 },
            { 1.0, 1.0, 1.0, -1.0 },
            { 1.0, -1.0, 1.0, 1.0 }
        };

        static const double dir[] = { -1.0, 1.0, -1.0, 1.0 };

        Vector4 f = Vector4::Zero();
        Vector4 z = getZRPY();
        Vector4 dz = (z - zprev) / dt;

        if(gyroSensor) {
            Vector3 w = ioBody->rootLink()->R() * gyroSensor->w();
            dz[3] = w[2];
        }

        Vector3 dv(0.0, 0.0, 9.80665);
        if(accSensor) {
//            dv = accSensor->dv();
        }

        Vector4 ddz = (dz - dzprev) / dt;

        Vector2 xy = getXY();
        Vector2 dxy = (xy - xyprev) / dt;
        Vector2 ddxy = (dxy - dxyprev) / dt;
        Vector2 dxy_local = Eigen::Rotation2Dd(-z[3]) * dxy;
        Vector2 ddxy_local = Eigen::Rotation2Dd(-z[3]) * ddxy;

        double cc = cos(z[1]) * cos(z[2]);
        double gfcoef = ioBody->mass() * dv[2] / 4.0 / cc ;

        if(!on) {
            zref[0] = 0.0;
            dzref[0] = 0.0;
        }

        for(int i = 0; i < 4; i++) {
            double pos = joystick->getPosition(targetMode, axisID[i]);
            if(fabs(pos) < 0.20) {
                pos = 0.0;
            }
            if(wailly && mode1 && (i == 0)) {
                pos *= -1.0;
            }

            if(i == 3) {
                dzref[i] = X[i] * pos;
                f[i] = P[i] * (dzref[i] - dz[i]) + D[i] * (0.0 - ddz[i]);
            } else {
                if(i == 0) {
                    zref[i] += X[i] * pos;
                } else {
                    if(manualMode) {
                        zref[i] = X[i] * pos;
                    } else {
                        int j = i - 1;
                        dxyref[j] = KX[j] * pos;
                        zref[i] = KP[j] * (dxyref[j] - dxy_local[1 - j])
                                + KD[j] * (0.0 - ddxy_local[1 - j]);
                    }
                }
                if(i == 1) {
                    zref[i] *= -1.0;
                }
                f[i] = P[i] * (zref[i] - z[i]) + D[i] * (0.0 - dz[i]);
            }
        }

        zprev = z;
        dzprev = dz;
        xyprev = xy;
        dxyprev = dxy;

        for(size_t i = 0; i < rotors.size(); i++) {
            Rotor* rotor = rotors[i];
            double force = 0.0;
            if(on) {
                force += gfcoef;
                force += sign[i][0] * f[0];
                force += sign[i][1] * f[1];
                force += sign[i][2] * f[2];
                force += sign[i][3] * f[3];
            }
            rotor->force() = force;
            rotor->torque() = dir[i] * force;
            rotor->notifyStateChange();
        }

        return true;
    }

    Vector4 getZRPY()
    {
        auto T = ioBody->rootLink()->position();
        double z = T.translation().z();
        Vector3 rpy = rpyFromRot(T.rotation());
        return Vector4(z, rpy[0], rpy[1], rpy[2]);
    }
    Vector2 getXY()
    {
        auto p = ioBody->rootLink()->translation();
        return Vector2(p.x(), p.y());
    }
};

CNOID_IMPLEMENT_SIMPLE_CONTROLLER_FACTORY(HobbyDroneJoystickController)
