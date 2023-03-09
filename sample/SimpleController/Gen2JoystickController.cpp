/**
   Gen2 Controller
   @author Kenta Suzuki
*/

#include <cnoid/EigenUtil>
#include <cnoid/JointPath>
#include <cnoid/SharedJoystick>
#include <cnoid/SimpleController>

using namespace cnoid;
using namespace std;

namespace {

const double joint_pose[] = {
    103.1, 197.3, 180.0, 43.6, 265.2, 257.5, 288.1, 0.0, 0.0, 0.0
};

}

class Gen2JoystickController : public SimpleController
{
    enum ControlID { TRANSLATION_MODE, WRIST_MODE, FINGER_MODE };
    enum JointID {
        SHOULDER, ARM_HALF_1, ARM_HALF_2,
        FOREARM, WRIST_SPHERICAL_1, WRIST_SPHERICAL_2,
        HAND_3FINGER, FINGER_PROXIMAL_1, FINGER_PROXIMAL_2,
        FINGER_PROXIMAL_3
    };

    SimpleControllerIO* io;
    Body* ioBody;
    BodyPtr ikBody;
    Link* ikWrist;
    shared_ptr<JointPath> baseToWrist;
    VectorXd qref, qref_old, qold;
    double dt;
    int controlId;
    bool isIKEnabled;
    bool isJointPoseSelected;
    bool prevButtonState[3];

    SharedJoystickPtr joystick;
    int targetMode;
    std::ostream* os;

public:

    virtual bool initialize(SimpleControllerIO* io) override
    {
        this->io = io;
        os = &io->os();
        ioBody = io->body();
        controlId = TRANSLATION_MODE;
        isIKEnabled = true;
        isJointPoseSelected = false;

        ikBody = ioBody->clone();
        ikWrist = ikBody->link("WRIST_ORIGIN");
        Link* base = ikBody->rootLink();
        baseToWrist = JointPath::getCustomPath(ikBody, base, ikWrist);
        base->p().setZero();
        base->R().setIdentity();

        for(int i = 0; i < ioBody->numJoints(); ++i) {
            Link* joint = ioBody->joint(i);
            if(!joint) {
                (*os) << "Joint " << i << " is not found." << endl;
                return false;
            }
            joint->setActuationMode(Link::JointVelocity);
            io->enableIO(joint);
        }

        initializeIK();

        dt = io->timeStep();

        joystick = io->getOrCreateSharedObject<SharedJoystick>("joystick");
        targetMode = joystick->addMode();

        return true;
    }

    virtual bool control() override
    {
        joystick->updateState(targetMode);

        static const int buttonID[] = { Joystick::A_BUTTON, Joystick::B_BUTTON, Joystick::LOGO_BUTTON };
        for(int i = 0; i < 3; ++i) {
            bool buttonState = joystick->getButtonState(targetMode, buttonID[i]);
            if(buttonState && !prevButtonState[i]) {
                if(i == 0) {
                    controlId = FINGER_MODE;
                    (*os) << "finger-mode has set." << endl;
                } else if(i == 1) {
                    if(controlId == TRANSLATION_MODE) {
                        controlId = WRIST_MODE;
                        (*os) << "wrist-mode has set." << endl;
                    } else {
                        controlId = TRANSLATION_MODE;
                        (*os) << "translation-mode has set." << endl;
                    }
                } else if(i == 2) {
                    isJointPoseSelected = true;
                }
            }
            prevButtonState[i] = buttonState;
        }

        isIKEnabled = true;
        if(controlId == FINGER_MODE) {
            isIKEnabled = false;
            double pos[3] = { 0.0 };
            double pos1[2];
            for(int i = 0; i < 2; ++i) {
                pos1[i] = joystick->getPosition(targetMode,
                            i == 0 ? Joystick::L_STICK_H_AXIS : Joystick::L_STICK_V_AXIS);
                if(fabs(pos1[i]) < 0.15) {
                    pos1[i] = 0.0;
                }
            }

            if(fabs(pos1[1]) < 0.15) {
                pos[0] = pos1[0];
                pos[1] = pos1[0];
                pos[2] = pos1[0];
            } else {
                pos[0] = pos1[1];
                pos[1] = pos1[1];
                pos[2] = 0.0;
            }

            static const int jointID[] = {
                FINGER_PROXIMAL_1, FINGER_PROXIMAL_2, FINGER_PROXIMAL_3
            };
            for(int i = 0; i < 3; ++i) {
                controlFK(jointID[i], pos[i]);
            }
        }

        doJointPose();
        
        if(isIKEnabled) {
            controlIK();
        } else {
            initializeIK();
        }

        return true;
    }

    void initializeIK()
    {
        const int nj = ioBody->numJoints();
        qold.resize(nj);
        for(int i = 0; i < nj; ++i) {
            Link* joint = ioBody->joint(i);
            double q = joint->q();
            ikBody->joint(i)->q() = q;
            qold[i] = q;
        }

        baseToWrist->calcForwardKinematics();
        qref = qold;
        qref_old = qold;
    }

    void controlFK(const int& jointId, double pos)
    {
        Link* joint = ioBody->joint(jointId);
        double q = joint->q();
        double q_lower = joint->q_lower();
        double q_upper = joint->q_upper();

        if((q > q_upper && pos > 0.0)
            || (q < q_lower && pos < 0.0)) {
            pos = 0.0;
        }

        joint->dq_target() = 1.06 * pos;
        qold[jointId] = q;
    }

    void controlIK()
    {
        VectorXd p(6);

        static const int axisID[] = {
            Joystick::L_STICK_H_AXIS, Joystick::L_STICK_V_AXIS, Joystick::DIRECTIONAL_PAD_H_AXIS
        };

        double pos[3];
        for(int i = 0; i < 3; ++i) {
            pos[i] = joystick->getPosition(targetMode, axisID[i]);
            if(fabs(pos[i]) < 0.15) {
                pos[i] = 0.0;
            }
        }

        if(controlId == TRANSLATION_MODE) {
            p.head<3>() = Vector3(-pos[0], pos[1], pos[2]) * 0.2 * dt;
        } else if(controlId == WRIST_MODE) {
            p.tail<3>() = degree(Vector3(pos[0], -pos[1], -pos[2])) * 1.06 * dt;
        }

        Isometry3 T;
        T.linear() = ikWrist->R() * rotFromRpy(radian(p.tail<3>()));
        T.translation() = ikWrist->p() + ikBody->rootLink()->R() * p.head<3>();
        if(baseToWrist->calcInverseKinematics(T)) {
            for(int i = 0; i < baseToWrist->numJoints(); ++i) {
                Link* joint = baseToWrist->joint(i);
                qref[joint->jointId()] = joint->q();
            }
        }

        for(int i = 0; i < ioBody->numJoints(); ++i) {
            Link* joint = ioBody->joint(i);
            double q = joint->q();
            double dq = (q - qold[i]) / dt;
            double dq_ref = (qref[i] - qref_old[i]) / dt;

            joint->dq_target() = dq_ref;
            qold[i] = q;
        }
        qref_old = qref;
    }

    void doJointPose()
    {
        double pos[10] = { 0.0 };
        bool changed = true;
        if(isJointPoseSelected) {
            for(int i = 0; i < ioBody->numJoints(); ++i) {
                Link* joint = ioBody->joint(i);
                double q = joint->q();
                double qref = radian(joint_pose[i]) - q;

                if(qref > 0.0) {
                    pos[i] = 1.0;
                } else if(qref < 0.0) {
                    pos[i] = -1.0;
                } else {
                    pos[i] = 0.0;
                }

                if(fabs(degree(qref)) < 1.0) {
                    pos[i] = 0.0;
                }

                if(fabs(pos[i]) < 0.15) {
                    pos[i] = 0.0;
                } else {
                    isIKEnabled = false;
                    changed = false;
                }
            }
            if(changed) {
                isJointPoseSelected = false;
            }
            for(int i = 0; i < ioBody->numJoints(); ++i) {
                controlFK(i, pos[i]);
            }
        }
    }
};

CNOID_IMPLEMENT_SIMPLE_CONTROLLER_FACTORY(Gen2JoystickController)