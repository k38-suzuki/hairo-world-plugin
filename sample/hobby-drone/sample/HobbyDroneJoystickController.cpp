/**
   HobbyDrone Controller
   @author Kenta Suzuki
*/

#include <cnoid/AccelerationSensor>
#include <cnoid/EigenUtil>
#include <cnoid/Joystick>
#include <cnoid/RateGyroSensor>
#include <cnoid/Rotor>
#include <cnoid/SimpleController>
#include <vector>

using namespace std;
using namespace cnoid;

class HobbyDroneJoystickController : public SimpleController
{
    enum {
        Mode1,
        Mode2
    };

    BodyPtr ioBody;
    DeviceList<Rotor> rotors;
    RateGyroSensor* gyroSensor;
    AccelerationSensor* accSensor;
    double timeStep;

    Vector4 zref;
    Vector4 zprev;
    Vector4 dzref;
    Vector4 dzprev;

    Vector2 xyref;
    Vector2 xyprev;
    Vector2 dxyref;
    Vector2 dxyprev;

    bool power;
    bool manualMode;
    int currentMode;

    struct ActionInfo {
        int actionId;
        int buttonId;
        bool prevButtonState;
        bool stateChanged;

        ActionInfo(int actionId, int buttonId)
            : actionId(actionId)
            , buttonId(buttonId)
            , prevButtonState(false)
            , stateChanged(false)
        {
        }
    };

    vector<ActionInfo> actions;

    Joystick* joystick;

public:
    virtual bool initialize(SimpleControllerIO* io) override
    {
        ioBody = io->body();
        timeStep = io->timeStep();
        rotors = io->body()->devices();
        gyroSensor = ioBody->findDevice<RateGyroSensor>("GyroSensor");
        accSensor = ioBody->findDevice<AccelerationSensor>("AccSensor");
        power = true;
        manualMode = false;
        currentMode = Mode1;

        string device = "/dev/input/js0";
        for(auto opt : io->options()) {
            if(opt == "manual") {
                manualMode = true;
            }
            if(opt == "mode2") {
                currentMode = Mode2;
            }
            if(opt == "/dev/input/js1") {
                device = "/dev/input/js1";
            }
        }

        io->enableInput(ioBody->rootLink(), LINK_POSITION);
        io->enableInput(gyroSensor);
        io->enableInput(accSensor);

        for(auto& rotor : rotors) {
            io->enableInput(rotor);
        }

        zref = zprev = getZRPY();
        dzref = dzprev = Vector4::Zero();
        xyref = xyprev = getXY();
        dxyref = dxyprev = Vector2::Zero();

        actions = {
            {0, Joystick::A_BUTTON},
            {1, Joystick::B_BUTTON}
        };

        joystick = new Joystick(device.c_str());

        return true;
    }

    virtual bool control() override
    {
        joystick->readCurrentState();

        for(auto& info : actions) {
            bool stateChanged = false;
            bool buttonState = joystick->getButtonState(info.buttonId);
            if(buttonState && !info.prevButtonState) {
                stateChanged = true;
            }
            info.prevButtonState = buttonState;
            if(stateChanged) {
                if(info.actionId == 0) {
                    power = !power;
                } else if(info.actionId == 1) {
                    currentMode = currentMode == Mode1 ? Mode2 : Mode1;
                }
            }
        }

        static const int modeID[][4] = {
            {Joystick::R_STICK_V_AXIS, Joystick::R_STICK_H_AXIS, Joystick::L_STICK_V_AXIS, Joystick::L_STICK_H_AXIS},
            {Joystick::L_STICK_V_AXIS, Joystick::R_STICK_H_AXIS, Joystick::R_STICK_V_AXIS, Joystick::L_STICK_H_AXIS}
        };

        int axisID[4];
        for(int i = 0; i < 4; i++) {
            axisID[i] = modeID[currentMode == Mode1 ? 0 : 1][i];
        }

        static const double P[] = {10.0, 1.0, 1.0, 0.01};
        static const double D[] = {5.0, 1.0, 1.0, 0.002};
        static const double X[] = {-0.001, 1.0, -1.0, -1.0};

        static const double KP[] = {1.0, 1.0};
        static const double KD[] = {1.0, 1.0};
        static const double KX[] = {-1.0, -1.0};

        static const double sign[4][4] = {
            {1.0, -1.0, -1.0, -1.0},
            {1.0,  1.0, -1.0,  1.0},
            {1.0,  1.0,  1.0, -1.0},
            {1.0, -1.0,  1.0,  1.0}
        };

        static const double dir[] = {-1.0, 1.0, -1.0, 1.0};

        Vector4 f = Vector4::Zero();
        Vector4 z = getZRPY();
        Vector4 dz = (z - zprev) / timeStep;

        if(gyroSensor) {
            Vector3 w = ioBody->rootLink()->R() * gyroSensor->w();
            dz[3] = w[2];
        }

        Vector3 dv(0.0, 0.0, 9.80665);
        if(accSensor) {
            //            dv = accSensor->dv();
        }

        Vector4 ddz = (dz - dzprev) / timeStep;

        Vector2 xy = getXY();
        Vector2 dxy = (xy - xyprev) / timeStep;
        Vector2 ddxy = (dxy - dxyprev) / timeStep;
        Vector2 dxy_local = Eigen::Rotation2Dd(-z[3]) * dxy;
        Vector2 ddxy_local = Eigen::Rotation2Dd(-z[3]) * ddxy;

        double cc = cos(z[1]) * cos(z[2]);
        double gfcoef = ioBody->mass() * dv[2] / 4.0 / cc;

        if((fabs(degree(z[1])) > 45.0) || (fabs(degree(z[2])) > 45.0)) {
            power = false;
        }

        if(!power) {
            zref[0] = 0.0;
            dzref[0] = 0.0;
        }

        double pos[4];
        for(int i = 0; i < 4; i++) {
            pos[i] = joystick->getPosition(axisID[i]);
            if(fabs(pos[i]) < 0.2) {
                pos[i] = 0.0;
            }

            if(i == 3) {
                dzref[i] = X[i] * pos[i];
                f[i] = P[i] * (dzref[i] - dz[i]) + D[i] * (0.0 - ddz[i]);
            } else {
                if(i == 0) {
                    zref[i] += X[i] * pos[i];
                } else {
                    if(manualMode) {
                        zref[i] = X[i] * pos[i];
                    } else {
                        int j = i - 1;
                        dxyref[j] = KX[j] * pos[i];
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
            if(power) {
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