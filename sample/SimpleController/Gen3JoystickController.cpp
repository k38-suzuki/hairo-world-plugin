/**
   Gen3 Controller
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
    0.0, -30.0, 0.0, 130.0, 0.0, -10.0, 0.0
};

}

class Gen3JoystickController : public SimpleController
{
    enum MapID { TWIST_LINEAR, TWIST_ANGULAR, JOINT };

    SimpleControllerIO* io;
    Body* ioBody;
    BodyPtr ikBody;
    Link* ikWrist;
    shared_ptr<JointPath> baseToWrist;
    VectorXd qref, qref_old, qold;
    double dt;
    bool isIKEnabled;
    bool isJointPoseSelected;
    bool prevButtonState[3];
    int controlMap;
    int currentJoint;
    int prevHAxis;
    int prevVAxis;
    int currentSpeed;

    SharedJoystickPtr joystick;
    int targetMode;
    std::ostream* os;

public:

    virtual bool initialize(SimpleControllerIO* io) override
    {
        this->io = io;
        os = &io->os();
        ioBody = io->body();
        isIKEnabled = true;
        isJointPoseSelected = false;
        for(int i = 0; i < 3; ++i) {
            prevButtonState[i] = false;
        }
        controlMap = TWIST_LINEAR;
        currentJoint = 0;
        prevHAxis = 0;
        prevVAxis = 0;
        currentSpeed = 50;

        ikBody = ioBody->clone();
        ikWrist = ikBody->link("WRIST_ORIGIN");
        Link* base = ikBody->rootLink();
        baseToWrist = JointPath::getCustomPath(base, ikWrist);
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

        static const char* texts[] = {
            "twist-linear control has set.", "twist-angular control has set.", "joint control has set."
        };
        static const int buttonID[] = { Joystick::B_BUTTON, Joystick::SELECT_BUTTON, Joystick::START_BUTTON };
        for(int i = 0; i < 3; ++i) {
            bool buttonState = joystick->getButtonState(targetMode, buttonID[i]);
            if(buttonState && !prevButtonState[i]) {
                if(i == 0) {
                    isJointPoseSelected = true;
                } else if(i == 1) {
                    --controlMap;
                    if(controlMap < 0) {
                        controlMap += 3;
                    }
                    (*os) << texts[controlMap] << endl;
                } else if(i == 2) {
                    controlMap = ++controlMap % 3;
                    (*os) << texts[controlMap] << endl;
                }
            }
            prevButtonState[i] = buttonState;
        }

        isIKEnabled = true;

        // speed
        {
            double pos = joystick->getPosition(targetMode, Joystick::DIRECTIONAL_PAD_V_AXIS);
            if(fabs(pos) < 0.15) {
                pos = 0.0;
            }
            if((int)pos != 0 && prevVAxis == 0) {
                if(pos == -1) {
                    currentSpeed += 10;
                    if(currentSpeed > 100) {
                        currentSpeed = 100;
                    }
                } else if(pos == 1) {
                    currentSpeed -= 10;
                    if(currentSpeed < 40) {
                        currentSpeed = 40;
                    }
                }
                (*os) << "current speed is " << currentSpeed << "%." << endl;
            }
            prevVAxis = (int)pos;
        }

        if(controlMap == JOINT) {
            // joint selection
            {
                double pos = joystick->getPosition(targetMode, Joystick::DIRECTIONAL_PAD_H_AXIS);
                if(fabs(pos) < 0.15) {
                    pos = 0.0;
                }
                if((int)pos != 0 && prevHAxis == 0) {
                    if(pos == -1) {
                        --currentJoint;
                        if(currentJoint < 0) {
                            currentJoint = 6;
                        }
                        (*os) << "joint " << currentJoint << " is selected." << endl;
                    } else if(pos == 1) {
                        ++currentJoint;
                        if(currentJoint > 6) {
                            currentJoint = 0;
                        }
                        (*os) << "joint " << currentJoint << " is selected." << endl;
                    }
                }
                prevHAxis = (int)pos;
            }

            isIKEnabled = false;
            double pos = joystick->getPosition(targetMode, Joystick::L_STICK_H_AXIS);
            if(fabs(pos) < 0.15) {
                pos = 0.0;
            }

            controlFK(currentJoint, pos);
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

        static const double speeds[] = {
            0.8727, 0.8727, 0.8727, 0.8727, 0.8727, 0.8727, 0.8727
        };

        joint->dq_target() = speeds[jointId] * (double)currentSpeed / 100.0 * pos;
        qold[jointId] = q;
    }

    void controlIK()
    {
        VectorXd p(6);

        static const int axisID[] = {
            Joystick::L_STICK_V_AXIS, Joystick::L_STICK_H_AXIS, Joystick::R_STICK_V_AXIS,
            Joystick::L_STICK_V_AXIS, Joystick::L_STICK_H_AXIS, Joystick::R_STICK_H_AXIS
        };

        double pos[6];
        for(int i = 0; i < 6; ++i) {
            pos[i] = joystick->getPosition(targetMode, axisID[i]);
            if(fabs(pos[i]) < 0.15) {
                pos[i] = 0.0;
            }
        }

        double rate = (double)currentSpeed / 100.0;
        if(controlMap == TWIST_LINEAR) {
            p.head<3>() = Vector3(-pos[0], -pos[1], -pos[2]) * 0.5 * rate * dt;
        } else if(controlMap == TWIST_ANGULAR) {
            p.tail<3>() = degree(Vector3(pos[3], -pos[4], -pos[5])) * 0.8727 * rate * dt;
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
        double pos[7] = { 0.0 };
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

CNOID_IMPLEMENT_SIMPLE_CONTROLLER_FACTORY(Gen3JoystickController)
