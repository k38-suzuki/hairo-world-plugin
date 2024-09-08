/**
    Gen2 Joint Map Controller
    Kenta Suzuki
 */

 #include <cnoid/SimpleController>
#include <cnoid/SharedJoystick>

 using namespace std;
 using namespace cnoid;

 class Gen2JointMapController : public SimpleController
 {
    SimpleControllerIO* io;
    int jointActuationMode;
    Link* armJoint[7];
    Link* fingerJoint[3];
    double dt;
    bool prevButtonState[2];
    int currentJoint;

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
        prevButtonState[0] = prevButtonState[1] = false;
        currentJoint = 0;
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

            } else if(opt == "Gen3") {

            } else if(opt == "Gen3lite") {

            }
        }

        armJoint[0] = body->link("SHOULDER");
        armJoint[1] = body->link("ARM_HALF_1");
        armJoint[2] = body->link("ARM_HALF_2");
        armJoint[3] = body->link("FOREARM");
        armJoint[4] = body->link("WRIST_SPHERICAL_1");
        armJoint[5] = body->link("WRIST_SPHERICAL_2");
        armJoint[6] = body->link("HAND_3FINGER");
        for(int i = 0; i < 7; ++i) {
            Link* joint = armJoint[i];
            if(!joint) {
                os << "Arm joint " << i << " is not found." << endl;
                return false;
            }
            // joint->q() = home_position[i];
            joint->setActuationMode(jointActuationMode);
            io->enableIO(joint);
        }

        fingerJoint[0] = body->link("FINGER_PROXIMAL_1");
        fingerJoint[1] = body->link("FINGER_PROXIMAL_2");
        fingerJoint[2] = body->link("FINGER_PROXIMAL_3");
        for(int i = 0; i < 3; ++i) {
            Link* joint = fingerJoint[i];
            if(!joint) {
                os << "Finger joint " << i << " is not found." << endl;
                return false;
            }
            joint->setActuationMode(jointActuationMode);
            io->enableIO(joint);
        }

        dt = io->timeStep();

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

        for(int i = 0; i < 3; ++i) {
            Link* joint = fingerJoint[i];
            double pos2 = pos[0] < 0.2 ? -pos[1] : pos[0];

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

        for(int i = 0; i < 2; ++i) {
            double pos = joystick->getPosition(targetMode,
                Joystick::DIRECTIONAL_PAD_H_AXIS);
            if(fabs(pos) < 0.2) {
                pos = 0.0;
            }

            bool currentState = i == 0 ? (pos < 0.0 ? true : false) : (pos > 0.0 ? true : false);
            if(currentState && !prevButtonState[i]) {
                if(i == 0) {
                    currentJoint = currentJoint == 0 ? 6 : currentJoint - 1;
                } else if(i == 1) {
                    currentJoint = currentJoint == 6 ? 0 : currentJoint + 1;
                }
                io->os() << "Current joint is " << currentJoint << ": " << armJoint[currentJoint]->name() << "." << endl;
            }
            prevButtonState[i] = currentState;
        }

        for(int i = 0; i < 7; ++i) {
            Link* joint = armJoint[i];
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

 CNOID_IMPLEMENT_SIMPLE_CONTROLLER_FACTORY(Gen2JointMapController)