/**
    Joint Map Controller
    Kenta Suzuki
*/

 #include <cnoid/SimpleController>
#include <cnoid/SharedJoystick>
#include <vector>

 using namespace std;
 using namespace cnoid;

 class JointMapController : public SimpleController
 {
    SimpleControllerIO* io;
    int jointActuationMode;
    vector<Link*> armJoints;
    vector<Link*> fingerJoints;
    double dt;
    int currentJoint;
    int robotType;
    int max_joints;

    enum RobotType { GEN2, GEN3, GEN3LITE };

    enum Gen2Joint {
        G2_SHOULDER,
        G2_ARM_HALF_1,
        G2_ARM_HALF_2,
        G2_FOREARM,
        G2_WRIST_SPHERICAL_1,
        G2_WRIST_SPHERICAL_2,
        G2_HAND_3FINGER,
        NUM_G2_JOINTS
    };

    enum Gen2Finger {
        G2_FINGER_PROXIMAL_1,
        G2_FINGER_PROXIMAL_2,
        G2_FINGER_PROXIMAL_3,
        NUM_G2_FINGERS
    };

    enum Gen3Joint {
        G3_J0,
        G3_J1,
        G3_J2,
        G3_J3,
        G3_J4,
        G3_J5,
        G3_J6,
        NUM_G3_JOINTS
    };

    enum Gen3liteJoint {
        G3L_J0,
        G3L_J1,
        G3L_J2,
        G3L_J3,
        G3L_J4,
        GRIPPER_FRAME,
        NUM_G3L_JOINTS
    };

    enum Gen3liteFinger {
        G3L_RIGHT_FINGER_PROX,
        G3L_RIGHT_FINGER_DIST,
        G3L_LEFT_FINGER_PROX,
        G3L_LEFT_FINGER_DIST,
        NUM_G3L_FINGERS
    };

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
        Body* body = io->body();

        // jointActuationMode = Link::JointEffort;
        jointActuationMode = Link::JointVelocity;
        armJoints.clear();
        fingerJoints.clear();
        currentJoint = 0;
        robotType = GEN2;
        for(auto opt : io->options()) {
            if(opt == "position") {
                jointActuationMode = Link::JointDisplacement;
                os << "The joint-position command mode is used." << endl;
            }
            if(opt == "velocity") {
                jointActuationMode = Link::JointVelocity;
                os << "The joint-velocity command mode is used." << endl;
            }
            if(opt == "Gen2") {
                robotType = GEN2;
                armJoints.resize(NUM_G2_JOINTS);
                fingerJoints.resize(NUM_G2_FINGERS);
                max_joints = NUM_G2_JOINTS - 1;
            } else if(opt == "Gen3") {
                robotType = GEN3;
                armJoints.resize(NUM_G3_JOINTS);
                fingerJoints.clear();
                max_joints = NUM_G3_JOINTS - 1;
            } else if(opt == "Gen3lite") {
                robotType = GEN3LITE;
                armJoints.resize(NUM_G3L_JOINTS);
                fingerJoints.resize(NUM_G3L_FINGERS);
                max_joints = NUM_G3L_JOINTS - 1;
            }
        }

        switch(robotType) {
            case GEN2:
                armJoints[0] = body->link("j2s7s300_joint_1");
                armJoints[1] = body->link("j2s7s300_joint_2");
                armJoints[2] = body->link("j2s7s300_joint_3");
                armJoints[3] = body->link("j2s7s300_joint_4");
                armJoints[4] = body->link("j2s7s300_joint_5");
                armJoints[5] = body->link("j2s7s300_joint_6");
                armJoints[6] = body->link("j2s7s300_joint_7");

                fingerJoints[0] = body->link("j2s7s300_joint_finger_1");
                fingerJoints[1] = body->link("j2s7s300_joint_finger_2");
                fingerJoints[2] = body->link("j2s7s300_joint_finger_3");
                break;
            case GEN3:
                armJoints[0] = body->link("joint_1");
                armJoints[1] = body->link("joint_2");
                armJoints[2] = body->link("joint_3");
                armJoints[3] = body->link("joint_4");
                armJoints[4] = body->link("joint_5");
                armJoints[5] = body->link("joint_6");
                armJoints[6] = body->link("joint_7");
                break;
            case GEN3LITE:
                armJoints[0] = body->link("joint_1");
                armJoints[1] = body->link("joint_2");
                armJoints[2] = body->link("joint_3");
                armJoints[3] = body->link("joint_4");
                armJoints[4] = body->link("joint_5");
                armJoints[5] = body->link("joint_6");

                fingerJoints[0] = body->link("right_finger_bottom_joint");
                fingerJoints[1] = body->link("right_finger_tip_joint");
                fingerJoints[2] = body->link("left_finger_bottom_joint");
                fingerJoints[3] = body->link("left_finger_tip_joint");
                break;
            default:
                break;
        }

        for(size_t i = 0; i < armJoints.size(); ++i) {
            Link* joint = armJoints[i];
            if(!joint) {
                os << "Arm joint " << i << " is not found." << endl;
                return false;
            }
            joint->setActuationMode(jointActuationMode);
            io->enableIO(joint);
        }

        for(size_t i = 0; i < fingerJoints.size(); ++i) {
            Link* joint = fingerJoints[i];
            if(!joint) {
                os << "Finger joint " << i << " is not found." << endl;
                return false;
            }
            joint->setActuationMode(jointActuationMode);
            io->enableIO(joint);
        }

        dt = io->timeStep();

        actions = {
            { 0, 0 },
        };

        joystick = io->getOrCreateSharedObject<SharedJoystick>("joystick");
        targetMode = joystick->addMode();

        os << "L_STICK_H_AXIS: Joint speeds." << endl;
        os << "DIRECTIONAL_PAD_H_AXIS: Previous/next joint." << endl;
        os << "L_TRIGGER_AXIS: Gripper close." << endl;
        os << "R_TRIGGER_AXIS: Gripper open." << endl;

        return true;
    }

    virtual bool control() override
    {
        joystick->updateState(targetMode);

        double pos[2];
        for(int i = 0; i < 2; ++i) {
            pos[i] = joystick->getPosition(targetMode,
                i == 0 ? Joystick::L_TRIGGER_AXIS : Joystick::R_TRIGGER_AXIS);
            if(fabs(pos[i]) < 0.2) {
                pos[i] = 0.0;
            }
        }

        for(size_t i = 0; i < fingerJoints.size(); ++i) {
            Link* joint = fingerJoints[i];
            double pos2 = pos[0] < 0.2 ? -pos[1] : pos[0];
            if(robotType == GEN3LITE) {
                if(i == 0) {
                    pos2 *= -1.0;
                } else if(i == 1 || i == 3) {
                    pos2 = 0.0;
                }
            }

            if(jointActuationMode == Link::JointDisplacement) {
                joint->q_target() = joint->q() + pos2 * dt;
            } else if(jointActuationMode == Link::JointVelocity) {
                joint->dq_target() = pos2;
            } else if(jointActuationMode == Link::JointEffort) {
                double q = joint->q();
                double q_upper = joint->q_upper();
                double q_lower = joint->q_lower();
            }
        }

        if(1) {
            double pos = joystick->getPosition(targetMode,
                Joystick::DIRECTIONAL_PAD_H_AXIS);
            if(fabs(pos) < 0.2) {
                pos = 0.0;
            }

            bool getPadState[1];
            getPadState[0] = fabs(pos) > 0 ? true : false;

            for(auto& info : actions) {
                bool stateChanged = false;
                bool buttonState = getPadState[info.buttonId];
                if(buttonState && !info.prevButtonState) {
                    stateChanged = true;
                }
                info.prevButtonState = buttonState;
                if(stateChanged) {
                    if(info.actionId == 0) {
                        if(pos == -1) {
                            currentJoint = currentJoint == 0 ? max_joints : currentJoint - 1;
                        } else if(pos == 1) {
                            currentJoint = currentJoint == max_joints ? 0 : currentJoint + 1;
                        }
                        io->os() << "Current joint is " << currentJoint << ": " << armJoints[currentJoint]->name() << "." << endl;
                    }
                }
            }
        }

        for(size_t i = 0; i < armJoints.size(); ++i) {
            Link* joint = armJoints[i];
            double pos = joystick->getPosition(targetMode,
                Joystick::L_STICK_H_AXIS);
            pos = i == currentJoint ? pos : 0.0;

            if(jointActuationMode == Link::JointDisplacement) {
                joint->q_target() = joint->q() + pos * dt;
            } else if(jointActuationMode == Link::JointVelocity) {
                joint->dq_target() = pos;
            } else if(jointActuationMode == Link::JointEffort) {
                double q = joint->q();
                double q_upper = joint->q_upper();
                double q_lower = joint->q_lower();
            }
        }

        return true;
    }
 };

 CNOID_IMPLEMENT_SIMPLE_CONTROLLER_FACTORY(JointMapController)