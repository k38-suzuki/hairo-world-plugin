/**
   HobbyDrone Controller
   @author Kenta Suzuki
*/

#include <cnoid/EigenUtil>
#include <cnoid/Joystick>
#include <cnoid/RateGyroSensor>
#include <cnoid/Rotor>
#include <cnoid/SimpleController>
#include <vector>

using namespace std;
using namespace cnoid;

namespace {

const double pgain[] = { 10.0, 1.0, 1.0, 0.01 };
const double dgain[] = { 5.0, 1.0, 1.0, 0.002 };

} // namespace

class HobbyDroneJoystickController : public SimpleController
{
    enum { Mode1, Mode2 };

    BodyPtr ioBody;
    DeviceList<Rotor> rotors;
    RateGyroSensor* gyroSensor;
    Vector4 zref, zold;
    Vector4 dzref, dzold;
    Vector2 xyref, xyold;
    Vector2 dxyref, dxyold;
    bool is_powered_on;
    bool manualMode;
    int currentMode;
    double timeStep;

    struct ActionInfo {
        int actionId;
        int buttonId;
        bool prevButtonState;
        bool stateChanged;

        ActionInfo(int actionId, int buttonId)
            : actionId(actionId),
              buttonId(buttonId),
              prevButtonState(false),
              stateChanged(false)
        {
        }
    };

    vector<ActionInfo> actions;

    Joystick* joystick;

public:
    virtual bool initialize(SimpleControllerIO* io) override;
    virtual bool control() override;
    Vector4 getZRPY();
    Vector2 getXY();
};

CNOID_IMPLEMENT_SIMPLE_CONTROLLER_FACTORY(HobbyDroneJoystickController)

bool HobbyDroneJoystickController::initialize(SimpleControllerIO* io)
{
    ioBody = io->body();
    rotors = io->body()->devices();
    gyroSensor = ioBody->findDevice<RateGyroSensor>("GyroSensor");
    is_powered_on = true;
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

    io->enableInput(ioBody->rootLink(), Link::LinkPosition);

    for(auto& rotor : rotors) {
        io->enableInput(rotor);
    }
    io->enableInput(gyroSensor);

    zref = zold = getZRPY();
    dzref = dzold = Vector4::Zero();
    xyref = xyold = getXY();
    dxyref = dxyold = Vector2::Zero();

    timeStep = io->timeStep();

    actions = {
        {0, Joystick::A_BUTTON},
        {1, Joystick::B_BUTTON}
    };

    joystick = new Joystick(device.c_str());

    return true;
}

bool HobbyDroneJoystickController::control()
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
                is_powered_on = !is_powered_on;
            } else if(info.actionId == 1) {
                currentMode = currentMode == Mode1 ? Mode2 : Mode1;
            }
        }
    }

    static const int axisID[][4] = {
        {Joystick::R_STICK_V_AXIS, Joystick::R_STICK_H_AXIS, Joystick::L_STICK_V_AXIS, Joystick::L_STICK_H_AXIS},
        {Joystick::L_STICK_V_AXIS, Joystick::R_STICK_H_AXIS, Joystick::R_STICK_V_AXIS, Joystick::L_STICK_H_AXIS}
    };

    double pos[4];
    for(int i = 0; i < 4; i++) {
        pos[i] = joystick->getPosition(axisID[currentMode == Mode1 ? 0 : 1][i]);
        if(fabs(pos[i]) < 0.2) {
            pos[i] = 0.0;
        }
    }

    Vector4 f = Vector4::Zero();
    Vector4 z = getZRPY();
    Vector4 dz = (z - zold) / timeStep;
    if(gyroSensor) {
        Vector3 w = ioBody->rootLink()->R() * gyroSensor->w();
        dz[3] = w[2];
    }
    Vector4 ddz = (dz - dzold) / timeStep;

    Vector2 xy = getXY();
    Vector2 dxy = (xy - xyold) / timeStep;
    Vector2 ddxy = (dxy - dxyold) / timeStep;
    Vector2 dxy_local = Eigen::Rotation2Dd(-z[3]) * dxy;
    Vector2 ddxy_local = Eigen::Rotation2Dd(-z[3]) * ddxy;

    double cc = cos(z[1]) * cos(z[2]);
    double gfcoef = ioBody->mass() * 9.80665 / 4.0 / cc;

    if((fabs(degree(z[1])) > 45.0) || (fabs(degree(z[2])) > 45.0)) {
        is_powered_on = false;
    }

    if(!is_powered_on) {
        zref[0] = 0.0;
        dzref[0] = 0.0;
    }

    static const double P = 1.0;
    static const double D = 1.0;

    for(int i = 0; i < 4; i++) {
        if(i == 3) {
            dzref[i] = -1.0 * pos[i];
            f[i] = (dzref[i] - dz[i]) * pgain[i] + (0.0 - ddz[i]) * dgain[i];
        } else {
            if(i == 0) {
                zref[i] += -0.001 * pos[i];
            } else {
                if(manualMode) {
                    zref[i] = (i == 1 ? 1.0 : -1.0) * pos[i];
                } else {
                    int j = i - 1;
                    dxyref[j] = -1.0 * pos[i];
                    zref[i] = P * (dxyref[j] - dxy_local[1 - j]) + D * (0.0 - ddxy_local[1 - j]);
                }
            }
            if(i == 1) {
                zref[i] *= -1.0;
            }
            f[i] = (zref[i] - z[i]) * pgain[i] + (0.0 - dz[i]) * dgain[i];
        }
    }
    zold = z;
    dzold = dz;
    xyold = xy;
    dxyold = dxy;

    static const double sign[4][4] = {
        {1.0, -1.0, -1.0, -1.0},
        {1.0,  1.0, -1.0,  1.0},
        {1.0,  1.0,  1.0, -1.0},
        {1.0, -1.0,  1.0,  1.0}
    };
    static const double dir[] = { -1.0, 1.0, -1.0, 1.0 };

    for(size_t i = 0; i < rotors.size(); i++) {
        Rotor* rotor = rotors[i];
        double force = is_powered_on
                           ? gfcoef + sign[i][0] * f[0] + sign[i][1] * f[1] + sign[i][2] * f[2] + sign[i][3] * f[3]
                           : 0.0;
        rotor->force() = force;
        rotor->torque() = dir[i] * force;
        rotor->notifyStateChange();
    }

    return true;
}

Vector4 HobbyDroneJoystickController::getZRPY()
{
    auto T = ioBody->rootLink()->position();
    double z = T.translation().z();
    Vector3 rpy = rpyFromRot(T.rotation());
    return Vector4(z, rpy[0], rpy[1], rpy[2]);
}

Vector2 HobbyDroneJoystickController::getXY()
{
    auto p = ioBody->rootLink()->translation();
    return Vector2(p.x(), p.y());
}