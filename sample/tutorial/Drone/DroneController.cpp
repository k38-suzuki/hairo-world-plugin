#include <cnoid/EigenUtil>
#include <cnoid/SimpleController>
#include <cnoid/SharedJoystick>
#include <cnoid/RateGyroSensor>
#include <cnoid/Rotor>

using namespace std;
using namespace cnoid;

class DroneController : public SimpleController
{
    enum ControlMode { Mode1, Mode2 };

    BodyPtr ioBody;
    DeviceList<Rotor> rotors;
    RateGyroSensor* gyroSensor;

    Vector4 zref;
    Vector4 zprev;
    Vector4 dzref;
    Vector4 dzprev;

    Vector2 xyref;
    Vector2 xyprev;
    Vector2 dxyref;
    Vector2 dxyprev;

    bool power;
    int currentMode;
    double timeStep;

    SharedJoystickPtr joystick;
    int targetMode;
    bool prevButtonState[2];

public:

    virtual bool initialize(SimpleControllerIO* io) override
    {
        ioBody = io->body();
        timeStep = io->timeStep();
        rotors = io->body()->devices();
        gyroSensor = ioBody->findDevice<RateGyroSensor>("GyroSensor");
        power = true;
        currentMode = Mode1;

        prevButtonState[0] = prevButtonState[1] = false;
        for(auto opt : io->options()) {
            if(opt == "mode2") {
                currentMode = Mode2;
            }
        }

        io->enableInput(ioBody->rootLink(), LINK_POSITION);
        io->enableInput(gyroSensor);

        for(auto& rotor : rotors) {
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
        static const int buttonID[] = { Joystick::A_BUTTON, Joystick::B_BUTTON };

        joystick->updateState(targetMode);

        for(int i = 0; i < 2; ++i) {
            bool currentState = joystick->getButtonState(targetMode, buttonID[i]);
            if(currentState && !prevButtonState[i]) {
                if(i == 0) {
                    power = !power;
                } else if(i == 1) {
                    currentMode = currentMode == Mode1 ? Mode2 : Mode1;
                }
            }
            prevButtonState[i] = currentState;
        }

        static const int modeID[][4] = {
            { Joystick::R_STICK_V_AXIS, Joystick::R_STICK_H_AXIS, Joystick::L_STICK_V_AXIS, Joystick::L_STICK_H_AXIS },
            { Joystick::L_STICK_V_AXIS, Joystick::R_STICK_H_AXIS, Joystick::R_STICK_V_AXIS, Joystick::L_STICK_H_AXIS }
        };

        int axisID[4] = { 0 };
        for(int i = 0; i < 4; i++) {
            axisID[i] = currentMode == Mode1 ? modeID[0][i] : modeID[1][i];
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
        Vector4 dz = (z - zprev) / timeStep;

        if(gyroSensor) {
            Vector3 w = ioBody->rootLink()->R() * gyroSensor->w();
            dz[3] = w[2];
        }

        Vector3 dv(0.0, 0.0, 9.80665);

        Vector4 ddz = (dz - dzprev) / timeStep;

        Vector2 xy = getXY();
        Vector2 dxy = (xy - xyprev) / timeStep;
        Vector2 ddxy = (dxy - dxyprev) / timeStep;
        Vector2 dxy_local = Eigen::Rotation2Dd(-z[3]) * dxy;
        Vector2 ddxy_local = Eigen::Rotation2Dd(-z[3]) * ddxy;

        double cc = cos(z[1]) * cos(z[2]);
        double gfcoef = ioBody->mass() * dv[2] / 4.0 / cc ;

        if((fabs(degree(z[1])) > 45.0) || (fabs(degree(z[2])) > 45.0)) {
            power = false;
        }

        if(!power) {
            zref[0] = 0.0;
            dzref[0] = 0.0;
        }

        double pos[4];
        for(int i = 0; i < 4; i++) {
            pos[i] = joystick->getPosition(targetMode, axisID[i]);
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
                    int j = i - 1;
                    dxyref[j] = KX[j] * pos[i];
                    zref[i] = KP[j] * (dxyref[j] - dxy_local[1 - j])
                            + KD[j] * (0.0 - ddxy_local[1 - j]);
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

CNOID_IMPLEMENT_SIMPLE_CONTROLLER_FACTORY(DroneController)