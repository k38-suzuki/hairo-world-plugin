/**
    SimpleUAV typeT Controller
    @author Kenta Suzuki
*/

#include <cnoid/EigenUtil>
#include <cnoid/Joystick>
#include <cnoid/RateGyroSensor>
#include <cnoid/Rotor>
#include <cnoid/SimpleController>

using namespace cnoid;

namespace {

const double pgain[] = { 0.10, 0.003, 0.003, 0.00002 };
const double dgain[] = { 0.05, 0.002, 0.002, 0.00001 };

} // namespace

class UAVtypeTJoystickController : public SimpleController
{
    enum { Mode1, Mode2 };

    BodyPtr ioBody;
    DeviceList<Rotor> rotors;
    RateGyroSensor* gyroSensor;
    Vector4 zref, zold;
    Vector4 dzref, dzold;
    Vector2 xref, xold;
    Vector2 dxref, dxold;
    int currentMode;
    double timeStep;

    Joystick joystick;

public:
    virtual bool initialize(SimpleControllerIO* io) override;
    virtual bool control() override;
    Vector4 getZRPY();
    Vector2 getXY();
};

CNOID_IMPLEMENT_SIMPLE_CONTROLLER_FACTORY(UAVtypeTJoystickController)

bool UAVtypeTJoystickController::initialize(SimpleControllerIO* io)
{
    ioBody = io->body();
    rotors = ioBody->devices();
    gyroSensor = ioBody->findDevice<RateGyroSensor>("GyroSensor");
    currentMode = Mode2;

    for(auto opt : io->options()) {
        if(opt == "mode1") {
            currentMode = Mode1;
        }
    }

    io->enableInput(ioBody->rootLink(), Link::LinkPosition);

    for(auto& rotor : rotors) {
        io->enableInput(rotor);
    }
    io->enableInput(gyroSensor);

    zref = zold = getZRPY();
    dzref = dzold = Vector4::Zero();
    xref = xold = getXY();
    dxref = dxold = Vector2::Zero();

    timeStep = io->timeStep();

    return true;
}

bool UAVtypeTJoystickController::control()
{
    joystick.readCurrentState();

    static const int axisID[][4] = {
        {Joystick::R_STICK_V_AXIS, Joystick::R_STICK_H_AXIS, Joystick::L_STICK_V_AXIS, Joystick::L_STICK_H_AXIS},
        {Joystick::L_STICK_V_AXIS, Joystick::R_STICK_H_AXIS, Joystick::R_STICK_V_AXIS, Joystick::L_STICK_H_AXIS}
    };

    double pos[4];
    for(int i = 0; i < 4; i++) {
        pos[i] = joystick.getPosition(axisID[currentMode == Mode1 ? 0 : 1][i]);
        if(fabs(pos[i]) < 0.2) {
            pos[i] = 0.0;
        }
    }

    Vector4 fz = Vector4::Zero();
    Vector4 z = getZRPY();
    Vector4 dz = (z - zold) / timeStep;
    if(gyroSensor) {
        Vector3 w = ioBody->rootLink()->R() * gyroSensor->w();
        dz[3] = w[2];
    }
    Vector4 ddz = (dz - dzold) / timeStep;

    Vector2 x = getXY();
    Vector2 dx = (x - xold) / timeStep;
    Vector2 ddx = (dx - dxold) / timeStep;
    Vector2 dx_local = Eigen::Rotation2Dd(-z[3]) * dx;
    Vector2 ddx_local = Eigen::Rotation2Dd(-z[3]) * ddx;

    double cc = cos(z[1]) * cos(z[2]);
    double gc = ioBody->mass() * 9.80665 / 4.0 / cc;

    if((fabs(degree(z[1])) > 45.0) || (fabs(degree(z[2])) > 45.0)) {
        return false;
    }

    // if (!power) {
    //     zref[0] = 0.0;
    //     dzref[0] = 0.0;
    // }

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
    zold = z;
    dzold = dz;
    xold = x;
    dxold = dx;

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

Vector4 UAVtypeTJoystickController::getZRPY()
{
    auto T = ioBody->rootLink()->position();
    double z = T.translation().z();
    Vector3 rpy = rpyFromRot(T.rotation());
    return Vector4(z, rpy[0], rpy[1], rpy[2]);
}

Vector2 UAVtypeTJoystickController::getXY()
{
    auto p = ioBody->rootLink()->translation();
    return Vector2(p.x(), p.y());
}