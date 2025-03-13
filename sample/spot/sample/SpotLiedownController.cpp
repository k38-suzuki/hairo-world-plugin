/**
    Spot Liedown Controller
    @author Kenta Suzuki
*/

#include <cnoid/EigenUtil>
#include <cnoid/SimpleController>
#include <cnoid/SharedJoystick>
#include <vector>

using namespace std;
using namespace cnoid;

namespace {

const double pgain[] = {
    150.00, 150.00, 150.00, 150.00, 150.00, 150.00,
    150.00, 150.00, 150.00, 150.00, 150.00, 150.00
};

const double dgain[] = {
    15.00, 15.00, 15.00, 15.00, 15.00, 15.00,
    15.00, 15.00, 15.00, 15.00, 15.00, 15.00
};

const double down[] = {
    0.0, 96.2, -143.4, 0.0, 96.2, -143.4,
    0.0, 96.2, -143.4, 0.0, 96.2, -143.4
};

const double up[] = {
    0.0, 60.4, -96.2, 0.0, 60.4, -96.2,
    0.0, 60.4, -96.2, 0.0, 60.4, -96.2
};

}

class SpotLiedownController : public SimpleController
{
    Body* ioBody;
    VectorXd qref, qold, qref_old;
    double timeStep;
    bool isKinematicsMode;

    struct ActionInfo {
        int actionId;
        int buttonId;
        bool prevButtonState;
        bool stateChanged;
        const double* angleMap;
        ActionInfo(int actionId, int buttonId, const double* angleMap)
            : actionId(actionId),
              buttonId(buttonId),
              prevButtonState(false),
              stateChanged(false),
              angleMap(angleMap)
        { }
    };
    vector<ActionInfo> actions;
    bool is_liedown_enabled;

    SharedJoystickPtr joystick;
    int targetMode;

public:

    virtual bool initialize(SimpleControllerIO* io) override
    {
        ioBody = io->body();

        isKinematicsMode = false;
        for(auto opt : io->options()) {
            if(opt == "kinematics") {
                isKinematicsMode = true;
            }
        }

        const int nj = ioBody->numJoints();
        qold.resize(nj);
        for(int i = 0; i < nj; ++i) {
            Link* joint = ioBody->joint(i);
            joint->setActuationMode(isKinematicsMode ? JointDisplacement : JointEffort);
            io->enableIO(joint);
            double q = joint->q();
            qold[i] = q;
        }

        qref = qold;
        qref_old = qold;

        timeStep = io->timeStep();

        actions = {
            { 0, Joystick::B_BUTTON, down },
            { 1, Joystick::X_BUTTON,   up }
        };
        is_liedown_enabled = false;

        joystick = io->getOrCreateSharedObject<SharedJoystick>("joystick");
        targetMode = joystick->addMode();

        return true;
    }

    virtual bool control() override
    {
        joystick->updateState(targetMode);

        for(auto& info : actions) {
            bool stateChanged = false;
            bool buttonState = joystick->getButtonState(targetMode, info.buttonId);
            if(buttonState && !info.prevButtonState) {
                stateChanged = true;
            }
            info.prevButtonState = buttonState;
            if(stateChanged) {
                is_liedown_enabled = info.actionId == 0 ? true: false;
            }
        }

        for(int i = 0; i < ioBody->numJoints(); ++i) {
            double qe = radian(actions[is_liedown_enabled ? 0 : 1].angleMap[i]);
            double q = ioBody->joint(i)->q();
            double deltaq = (qe - q) * timeStep;
            qref[i] += deltaq;
        }

        for(int i = 0; i < ioBody->numJoints(); ++i) {
            if(isKinematicsMode) {
                ioBody->joint(i)->q_target() = qref[i];
            } else {
                double q = ioBody->joint(i)->q();
                double dq = (q - qold[i]) / timeStep;
                double dq_ref = (qref[i] - qref_old[i]) / timeStep;
                ioBody->joint(i)->u() = (qref[i] - q) * pgain[i] + (dq_ref - dq) * dgain[i];
                qold[i] = q;
            }
        }
        qref_old = qref;

        return true;
    }
};

CNOID_IMPLEMENT_SIMPLE_CONTROLLER_FACTORY(SpotLiedownController)