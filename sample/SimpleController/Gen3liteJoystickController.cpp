/**
   Gen3 lite Controller
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
    0.0, -20.0, 90.0, 80.0, 60.0, 0.0, 20.0, 0.0, -20.0, 0.0
};

const char* texts[] = {
    "twist-linear control has set.", "twist-angular control has set.", "joint control has set."
};

}

class Gen3liteJoystickController : public SimpleController
{
    enum ControlMap { TwistLinear, TwistAngular, Joint };

    SimpleControllerIO* io;
    int jointActuationMode;

    Body* ioBody;
    Link* ioRightFinger;
    Link* ioLeftFinger;
    BodyPtr ikBody;
    Link* ikWrist;
    shared_ptr<JointPath> baseToWrist;
    VectorXd qref, qold, qref_old;
    Interpolator<VectorXd> wristInterpolator;
    Interpolator<VectorXd> jointInterpolator;
    int phase;
    double time;
    double timeStep;
    double dq_hand[2];
    int controlMap;
    int currentJoint;
    int currentSpeed;

    SharedJoystickPtr joystick;
    int targetMode;
    bool prevButtonState[3];
    bool prevButtonState2[2];

public:

    virtual bool initialize(SimpleControllerIO* io) override
    {
        this->io = io;
        ostream& os = io->os();
        ioBody = io->body();

        ioRightFinger = ioBody->link("RIGHT_FINGER_PROX");
        ioLeftFinger = ioBody->link("LEFT_FINGER_PROX");

        prevButtonState[0] = prevButtonState[1] = prevButtonState[2] = false;
        prevButtonState2[0] = prevButtonState2[1] = false;
        jointActuationMode = Link::JointVelocity;
        for(auto opt : io->options()) {
            if(opt == "position") {
                jointActuationMode = Link::JointDisplacement;
                os << "The joint-position command mode is used." << endl;
            }
        }

        ikBody = ioBody->clone();
        ikWrist = ikBody->link("WRIST_ORIGIN");
        Link* base = ikBody->rootLink();
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

        VectorXd p0(6);
        p0.head<3>() = ikWrist->p();
        p0.tail<3>() = rpyFromRot(ikWrist->R());
        
        VectorXd p1(6);
        p1.head<3>() = ikWrist->p();
        p1.tail<3>() = rpyFromRot(ikWrist->R());
        
        wristInterpolator.clear();
        wristInterpolator.appendSample(0.0, p0);
        wristInterpolator.appendSample(0.1, p1);
        wristInterpolator.update();

        phase = 0;
        time = 0.0;
        timeStep = io->timeStep();
        dq_hand[0] = dq_hand[1] = 0.0;

        controlMap = TwistLinear;
        currentJoint = 0;
        currentSpeed = 50;
        joystick = io->getOrCreateSharedObject<SharedJoystick>("joystick");
        targetMode = joystick->addMode();

        return true;
    }

    virtual bool control() override
    {
       static const int axisID[] = {
            Joystick::L_STICK_H_AXIS, Joystick::L_STICK_V_AXIS, Joystick::R_STICK_H_AXIS,
            Joystick::R_STICK_V_AXIS, Joystick::DIRECTIONAL_PAD_H_AXIS, Joystick::DIRECTIONAL_PAD_V_AXIS,
            Joystick::L_TRIGGER_AXIS, Joystick::R_TRIGGER_AXIS
        };
        static const int buttonID[] = { Joystick::B_BUTTON, Joystick::SELECT_BUTTON, Joystick::START_BUTTON };

        joystick->updateState(targetMode);

        double pos[8];
        for(int i = 0; i < 8; ++i) {
            pos[i] = joystick->getPosition(targetMode, axisID[i]);
            if(fabs(pos[i]) < 0.2) {
                pos[i] = 0.0;
            }
        }

        for(int i = 0; i < 3; ++i) {
            bool currentState = joystick->getButtonState(targetMode, buttonID[i]);
            if(currentState && !prevButtonState[i]) {
                if(i == 0) {
                    // home position
                } else if(i == 1) {
                    controlMap = controlMap == 0 ? 2 : controlMap - 1;
                    io->os() << texts[controlMap] << endl;
                } else if(i == 2) {
                    controlMap = controlMap == 2 ? 0 : controlMap + 1;
                    io->os() << texts[controlMap] << endl;
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
        if(controlMap == Joint) {
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

        double rate = (double)currentSpeed / 100.0;

        VectorXd p(6);

        if(phase <= 2) {
            p = wristInterpolator.interpolate(time);
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

        if(phase == 0) {
            if(time > wristInterpolator.domainUpper()) {
                phase = 1;
            }
        } else if(phase == 1) {
            if(fabs(pos[6]) > 0 || fabs(pos[7]) > 0) {
                if(fabs(pos[6]) > fabs(pos[7])) {
                    dq_hand[0] = degree(pos[6] * 1.0 * timeStep) * rate * -1.0;
                    dq_hand[1] = degree(pos[6] * 1.0 * timeStep) * rate;
                } else {
                    dq_hand[0] = degree(pos[7] * 1.0 * timeStep) * rate;
                    dq_hand[1] = degree(pos[7] * 1.0 * timeStep) * rate * -1.0;
                }
                qref[ioLeftFinger->jointId()] += radian(dq_hand[0]);
                qref[ioRightFinger->jointId()] += radian(dq_hand[1]);

                // jointInterpolator.clear();
                // jointInterpolator.appendSample(time, qref);
                // VectorXd qf = VectorXd::Zero(qref.size());
                // qf[ioLeftFinger->jointId()] = qref[ioLeftFinger->jointId()];
                // qf[ioRightFinger->jointId()] = qref[ioRightFinger->jointId()];
                // jointInterpolator.appendSample(time + timeStep, qf);
                // jointInterpolator.update();
            } else {
                if(controlMap == Joint) {
                    // now editing...
                    // rotate by pos[0]
                    // rate of joint 5 = 1.57
                    // rate of other joint = 1.0
                } else {
                    VectorXd p0(6);
                    p0.head<3>() = ikWrist->p();
                    p0.tail<3>() = rpyFromRot(ikWrist->R());

                    VectorXd p1(6);
                    if(controlMap == TwistLinear) {
                        p1.head<3>() = ikWrist->p() + Vector3(-pos[1], -pos[0], -pos[3]) * 0.5 * timeStep * rate;
                        p1.tail<3>() = rpyFromRot(ikWrist->R());
                    } else if(controlMap == TwistAngular) {
                        p1.head<3>() = ikWrist->p();
                        p1.tail<3>() = rpyFromRot(ikWrist->R() * rotFromRpy(Vector3(pos[1], -pos[0], -pos[2]) * 1.0 * timeStep * rate));
                    }

                    wristInterpolator.clear();
                    wristInterpolator.appendSample(time + 0.0, p0);
                    wristInterpolator.appendSample(time + timeStep, p1);
                    wristInterpolator.update();
                }          
            }
            phase = 2;
        } else if(phase == 2) {
            // if(time > jointInterpolator.domainUpper()) {
            //     phase = 0;
            // }
            if(time > wristInterpolator.domainUpper()) {
                phase = 0;
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

CNOID_IMPLEMENT_SIMPLE_CONTROLLER_FACTORY(Gen3liteJoystickController)
