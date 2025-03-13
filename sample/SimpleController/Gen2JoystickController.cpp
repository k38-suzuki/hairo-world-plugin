/**
   Gen2 Controller
   @author Kenta Suzuki
*/

#include <cnoid/EigenUtil>
#include <cnoid/JointPath>
#include <cnoid/SharedJoystick>
#include <cnoid/SimpleController>
#include <sample/SimpleController/Interpolator.h>
#include <vector>

using namespace std;
using namespace cnoid;

namespace {

const double home_position[] = {
    103.1, 197.3, 180.0, 43.6, 265.2, 257.5, 288.1, 0.0, 0.0, 0.0
};

}

class Gen2JoystickController : public SimpleController
{
    enum ControlMode { TranslationMode, WristMode, FingersMode };

    SimpleControllerIO* io;
    int jointActuationMode;

    Body* ioBody;
    Link* ioFinger1;
    Link* ioFinger2;
    Link* ioFinger3;
    BodyPtr ikBody;
    Link* ikWrist;
    shared_ptr<JointPath> baseToWrist;
    VectorXd qref, qold, qref_old;
    Interpolator<VectorXd> jointInterpolator;
    double time;
    double timeStep;
    double dq_hand[3];

    int currentMode;
    bool is_pose_enabled;

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
        { }
    };
    vector<ActionInfo> actions;

    SharedJoystickPtr joystick;
    int targetMode;

public:

    virtual bool initialize(SimpleControllerIO* io) override
    {
        this->io = io;
        ostream& os = io->os();
        ioBody = io->body();

        jointActuationMode = Link::JointVelocity;
        string prefix;

        for(auto opt : io->options()) {
            if(opt == "position") {
                jointActuationMode = Link::JointDisplacement;
                os << "The joint-position command mode is used." << endl;
            } else {
                prefix = opt;
                io->os() << "prefix: " << prefix << endl;
            }
        }

        ioFinger1 = ioBody->link(prefix + "j2s7s300_joint_finger_1");
        ioFinger2 = ioBody->link(prefix + "j2s7s300_joint_finger_2");
        ioFinger3 = ioBody->link(prefix + "j2s7s300_joint_finger_3");

        ikBody = ioBody->clone();
        ikWrist = ikBody->link(prefix + "j2s7s300_joint_end_effector");
        Link* base = ikBody->link(prefix + "j2s7s300_joint_base");
        baseToWrist = JointPath::getCustomPath(base, ikWrist);
        base->p().setZero();
        base->R().setIdentity();

        const int nj = ioBody->numJoints();
        qold.resize(nj);
        for(int i = 0; i < nj; ++i) {
            Link* joint = ioBody->joint(i);
            joint->setActuationMode(jointActuationMode);
            io->enableIO(joint);
            double q = joint->q();
            ikBody->joint(i)->q() = q;
            qold[i] = q;
        }

        baseToWrist->calcForwardKinematics();
        qref = qold;
        qref_old = qold;

        time = 0.0;
        timeStep = io->timeStep();
        dq_hand[0] = dq_hand[1] = dq_hand[2] = 0.0;

        currentMode = TranslationMode;
        is_pose_enabled = false;

        actions = {
            { 0, Joystick::A_BUTTON    },
            { 1, Joystick::B_BUTTON    },
            { 2, Joystick::LOGO_BUTTON }
        };

        joystick = io->getOrCreateSharedObject<SharedJoystick>("joystick");
        targetMode = joystick->addMode();

        if(timeStep < 0.01) {
            os << "timestep < 0.01" << endl;
            return false;
        }

        return true;
    }

    virtual bool control() override
    {
        static const int axisID[] = {
            Joystick::L_STICK_H_AXIS, Joystick::L_STICK_V_AXIS, Joystick::DIRECTIONAL_PAD_H_AXIS
        };

        joystick->updateState(targetMode);

        double pos[3];
        for(int i = 0; i < 3; ++i) {
            pos[i] = joystick->getPosition(targetMode, axisID[i]);
            if(fabs(pos[i]) < 0.2) {
                pos[i] = 0.0;
            }
        }

        for(auto& info :actions) {
            bool stateChanged = false;
            bool buttonState = joystick->getButtonState(targetMode, info.buttonId);
            if(buttonState && !info.prevButtonState) {
                stateChanged = true;
            }
            info.prevButtonState = buttonState;
            if(stateChanged) {
                if(info.actionId == 0) {
                    currentMode = FingersMode;
                    io->os() << "fingers-mode has set." << endl;
                } else if(info.actionId == 1) {
                    currentMode = currentMode == TranslationMode ? WristMode : TranslationMode;
                    if(currentMode == TranslationMode) {
                        io->os() << "translation-mode has set." << endl;
                    } else {
                        io->os() << "wrist-mode has set." << endl;
                    }
                } else if(info.actionId == 2) {
                    // home position
                    jointInterpolator.clear();
                    jointInterpolator.appendSample(time, qref);
                    // VectorXd qf = VectorXd::Zero(qref.size());
                    VectorXd qf = qref;
                    for(int i = 0; i < ioBody->numJoints(); ++i) {
                        qf[i] = radian(home_position[i]);
                    }
                    qf[ioFinger1->jointId()] = qref[ioFinger1->jointId()];
                    qf[ioFinger2->jointId()] = qref[ioFinger2->jointId()];
                    qf[ioFinger3->jointId()] = qref[ioFinger3->jointId()];
                    jointInterpolator.appendSample(time + 2.0, qf);
                    jointInterpolator.update();
                    is_pose_enabled = true;
                }
            }
        }

        if(!is_pose_enabled) {
            if(currentMode == FingersMode) {
                if(fabs(pos[0]) > fabs(pos[1])) {
                    dq_hand[0] = dq_hand[1] = dq_hand[2] = degree(pos[0]) * 1.06 * timeStep;
                } else {
                    dq_hand[0] = dq_hand[1] = degree(pos[1]) * 1.06 * timeStep;
                    dq_hand[2] = 0.0;
                }

                if((ioFinger1->q() <= ioFinger1->q_lower() && pos[0] < 0.0)
                    || (ioFinger1->q() >= ioFinger1->q_upper() && pos[0] > 0.0)
                    || (ioFinger1->q() <= ioFinger1->q_lower() && pos[1] < 0.0)
                    || (ioFinger1->q() >= ioFinger1->q_upper() && pos[1] > 0.0)) {
                    dq_hand[0] = 0.0;
                }
                if((ioFinger2->q() <= ioFinger2->q_lower() && pos[0] < 0.0)
                    || (ioFinger2->q() >= ioFinger2->q_upper() && pos[0] > 0.0)
                    || (ioFinger2->q() <= ioFinger2->q_lower() && pos[1] < 0.0)
                    || (ioFinger2->q() >= ioFinger2->q_upper() && pos[1] > 0.0)) {
                    dq_hand[1] = 0.0;
                }
                if((ioFinger3->q() <= ioFinger3->q_lower() && pos[0] < 0.0)
                    || (ioFinger3->q() >= ioFinger3->q_upper() && pos[0] > 0.0)) {
                    dq_hand[2] = 0.0;
                }
                qref[ioFinger1->jointId()] += radian(dq_hand[0]);
                qref[ioFinger2->jointId()] += radian(dq_hand[1]);
                qref[ioFinger3->jointId()] += radian(dq_hand[2]);
            } else {
                VectorXd p3(6);
                if(currentMode == TranslationMode) {
                    p3.head<3>() = ikWrist->p() + Vector3(-pos[0], pos[1], pos[2]) * 0.2 * timeStep;
                    p3.tail<3>() = rpyFromRot(ikWrist->R());
                } else if(currentMode == WristMode) {
                    p3.head<3>() = ikWrist->p();
                    p3.tail<3>() = rpyFromRot(ikWrist->R() * rotFromRpy(Vector3(pos[0], -pos[1], -pos[2]) * 1.06 * timeStep));
                }

                VectorXd p(6);

                p = p3;
                Isometry3 T;
                T.linear() = rotFromRpy(Vector3(p.tail<3>()));
                T.translation() = p.head<3>();
                if(baseToWrist->calcInverseKinematics(T)) {
                    for(int i = 0; i < baseToWrist->numJoints(); ++i) {
                        Link* joint = baseToWrist->joint(i);
                        qref[joint->jointId()] = joint->q();
                    }
                }
            }
        } else {
            qref = jointInterpolator.interpolate(time);
            if(time > jointInterpolator.domainUpper()) {
                for(int i = 0; i < ioBody->numJoints(); ++i) {
                    Link* joint = ioBody->joint(i);
                    double q = joint->q();
                    ikBody->joint(i)->q() = q;
                    qold[i] = q;
                }

                baseToWrist->calcForwardKinematics();
                is_pose_enabled = false;
            }
        }

        for(int i = 0; i < ioBody->numJoints(); ++i) {
            if(jointActuationMode == Link::JointDisplacement) {
                ioBody->joint(i)->q_target() = qref[i];
            } else if(jointActuationMode == Link::JointVelocity) {
                double q = ioBody->joint(i)->q();
                double dq = (q - qold[i]) / timeStep;
                double dq_ref = (qref[i] - qref_old[i]) / timeStep;
                ioBody->joint(i)->dq_target() = dq_ref;
                qold[i] = q;
            }
        }
        qref_old = qref;
        time += timeStep;

        return true;
    }
};

CNOID_IMPLEMENT_SIMPLE_CONTROLLER_FACTORY(Gen2JoystickController)