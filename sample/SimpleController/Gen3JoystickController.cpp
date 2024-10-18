/**
   Gen3 Controller
   @author Kenta Suzuki
*/

#include <cnoid/EigenUtil>
#include <cnoid/JointPath>
#include <cnoid/SharedJoystick>
#include <cnoid/SimpleController>
#include <sample/SimpleController/Interpolator.h>

using namespace std;
using namespace cnoid;

namespace {

const double home_position[] = {
    360.0, 15.0, 180.0, 230.0, 360.0, 55.0, 90.0
};

}

class Gen3JoystickController : public SimpleController
{
    enum ControlMap { TwistLinear, TwistAngular, Joint };

    SimpleControllerIO* io;
    int jointActuationMode;

    Body* ioBody;
    BodyPtr ikBody;
    Link* ikWrist;
    shared_ptr<JointPath> baseToWrist;
    VectorXd qref, qold, qref_old;
    Interpolator<VectorXd> jointInterpolator;
    double time;
    double timeStep;
    double dq_hand[2];

    int currentMap;
    int currentJoint;
    int currentSpeed;
    bool is_pose_enabled;

    SharedJoystickPtr joystick;
    int targetMode;
    bool prevButtonState[3];
    bool prevButtonState2[2];
    bool prevMapState;

public:

    virtual bool initialize(SimpleControllerIO* io) override
    {
        this->io = io;
        ostream& os = io->os();
        ioBody = io->body();

        prevButtonState[0] = prevButtonState[1] = prevButtonState[2] = false;
        prevButtonState2[0] = prevButtonState2[1] = false;
        prevMapState = false;
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

        ikBody = ioBody->clone();
        ikWrist = ikBody->link(prefix + "end_effector");
        Link* base = ikBody->link(prefix + "base_joint");
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
        dq_hand[0] = dq_hand[1] = 0.0;

        currentMap = TwistLinear;
        currentJoint = 0;
        currentSpeed = 50;
        is_pose_enabled = false;

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
            Joystick::L_STICK_H_AXIS, Joystick::L_STICK_V_AXIS, Joystick::R_STICK_H_AXIS,
            Joystick::R_STICK_V_AXIS, Joystick::DIRECTIONAL_PAD_H_AXIS, Joystick::DIRECTIONAL_PAD_V_AXIS
        };
        static const int buttonID[] = { Joystick::B_BUTTON, Joystick::SELECT_BUTTON, Joystick::START_BUTTON };

        joystick->updateState(targetMode);

        double pos[6];
        for(int i = 0; i < 6; ++i) {
            pos[i] = joystick->getPosition(targetMode, axisID[i]);
            if(fabs(pos[i]) < 0.2) {
                pos[i] = 0.0;
            }
        }

        static const char* texts[] = {
            "twist-linear control has set.", "twist-angular control has set.",
            "joint control has set."
        };

        for(int i = 0; i < 3; ++i) {
            bool currentState = joystick->getButtonState(targetMode, buttonID[i]);
            if(currentState && !prevButtonState[i]) {
                if(i == 0) {
                    // home position
                    jointInterpolator.clear();
                    jointInterpolator.appendSample(time, qref);
                    // VectorXd qf = VectorXd::Zero(qref.size());
                    VectorXd qf = qref;
                    for(int i = 0; i < ioBody->numJoints(); ++i) {
                        qf[i] = radian(home_position[i]);
                    }
                    jointInterpolator.appendSample(time + 2.0, qf);
                    jointInterpolator.update();
                    is_pose_enabled = true;
                } else if(i == 1) {
                    currentMap = currentMap == 0 ? 2 : currentMap - 1;
                    io->os() << texts[currentMap] << endl;
                } else if(i == 2) {
                    currentMap = currentMap == 2 ? 0 : currentMap + 1;
                    io->os() << texts[currentMap] << endl;
                }
            }
            prevButtonState[i] = currentState;
        }

        // speed selection
        bool currentState1 = fabs(pos[5]) > 0 ? true : false;
        if(currentState1 && !prevButtonState2[0]) {
            if(pos[5] == -1) {
                currentSpeed = currentSpeed == 100 ? 100 : currentSpeed + 10;
            } else if(pos[5] == 1) {
                currentSpeed = currentSpeed == 40 ? 40 : currentSpeed - 10;
            }
            io->os() << "current speed is " << currentSpeed << "%." << endl;
        }
        prevButtonState2[0] = currentState1;

        // joint selection
        if(currentMap == Joint) {
            bool currentState2 = fabs(pos[4]) > 0 ? true : false;
            if(currentState2 && !prevButtonState2[1]) {
                if(pos[4] == -1) {
                    currentJoint = currentJoint == 0 ? 5 : currentJoint - 1;
                    io->os() << "joint " << currentJoint << " is selected." << endl;
                } else if(pos[4] == 1) {
                    currentJoint = currentJoint == 5 ? 0 : currentJoint + 1;
                    io->os() << "joint " << currentJoint << " is selected." << endl;
                }
            }
            prevButtonState2[1] = currentState2;
        }

        const double rate = (double)currentSpeed / 100.0;

        if(currentMap != prevMapState) {
            for(int i = 0; i < ioBody->numJoints(); ++i) {
                Link* joint = ioBody->joint(i);
                double q = joint->q();
                ikBody->joint(i)->q() = q;
                qold[i] = q;
            }

            baseToWrist->calcForwardKinematics();
        }
        prevMapState = currentMap;

        if(!is_pose_enabled) {
            if(currentMap == Joint) {
                Link* joint = ioBody->joint(currentJoint);
                if((joint->q() <= joint->q_lower() && pos[0] < 0.0)
                    || (joint->q() >= joint->q_upper() && pos[0] > 0.0)) {
                    pos[0] = 0.0;
                }

                // selected joint rotation
                qref[currentJoint] += pos[0] * 0.8727 * timeStep * rate;
            } else {
                VectorXd p3(6);
                if(currentMap == TwistLinear) {
                    p3.head<3>() = ikWrist->p() + Vector3(-pos[1], -pos[0], -pos[3]) * 0.5 * rate * timeStep;
                    p3.tail<3>() = rpyFromRot(ikWrist->R());
                } else if(currentMap == TwistAngular) {
                    p3.head<3>() = ikWrist->p();
                    p3.tail<3>() = rpyFromRot(ikWrist->R() * rotFromRpy(Vector3(pos[1], -pos[0], -pos[2]) * 1.0 * rate * timeStep));
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

CNOID_IMPLEMENT_SIMPLE_CONTROLLER_FACTORY(Gen3JoystickController)
