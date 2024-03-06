/**
   Gen2 Controller
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
    Interpolator<VectorXd> wristInterpolator;
    Interpolator<VectorXd> jointInterpolator;
    int phase;
    double time;
    double timeStep;
    double dq_hand[3];
    int controlMode;

    SharedJoystickPtr joystick;
    int targetMode;
    bool prevButtonState[3];

public:

    virtual bool initialize(SimpleControllerIO* io) override
    {
        this->io = io;
        ostream& os = io->os();
        ioBody = io->body();

        ioFinger1 = ioBody->link("FINGER_PROXIMAL_1");
        ioFinger2 = ioBody->link("FINGER_PROXIMAL_2");
        ioFinger3 = ioBody->link("FINGER_PROXIMAL_3");

        prevButtonState[0] = prevButtonState[1] = prevButtonState[2] = false;
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

        VectorXd p2(6);
        p2.head<3>() = ikWrist->p();
        p2.tail<3>() = rpyFromRot(ikWrist->R());
        
        VectorXd p3(6);
        p3.head<3>() = ikWrist->p();
        p3.tail<3>() = rpyFromRot(ikWrist->R());
        
        wristInterpolator.clear();
        wristInterpolator.appendSample(0.0, p2);
        wristInterpolator.appendSample(0.1, p3);
        wristInterpolator.update();

        phase = 0;
        time = 0.0;
        timeStep = io->timeStep();
        dq_hand[0] = dq_hand[1] = dq_hand[2] = 0.0;

        controlMode = TranslationMode;
        joystick = io->getOrCreateSharedObject<SharedJoystick>("joystick");
        targetMode = joystick->addMode();

        return true;
    }

    virtual bool control() override
    {
        static const int axisID[] = {
            Joystick::L_STICK_H_AXIS, Joystick::L_STICK_V_AXIS, Joystick::DIRECTIONAL_PAD_H_AXIS
        };
        static const int buttonID[] = { Joystick::A_BUTTON, Joystick::B_BUTTON, Joystick::LOGO_BUTTON };

        joystick->updateState(targetMode);

        double pos[3];
        for(int i = 0; i < 3; ++i) {
            pos[i] = joystick->getPosition(targetMode, axisID[i]);
            if(fabs(pos[i]) < 0.2) {
                pos[i] = 0.0;
            }
        }

        for(int i = 0; i < 3; ++i) {
            bool currentState = joystick->getButtonState(targetMode, buttonID[i]);
            if(currentState && !prevButtonState[i]) {
                if(i == 0) {
                    controlMode = FingersMode;
                    io->os() << "fingers-mode has set." << endl;
                } else if(i == 1) {
                    controlMode = controlMode == TranslationMode ? WristMode : TranslationMode;
                    if(controlMode == TranslationMode) {
                        io->os() << "translation-mode has set." << endl;
                    } else {
                        io->os() << "wrist-mode has set." << endl;
                    }
                } else if(i == 2) {
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
                    phase = 3;
                }
            }
            prevButtonState[i] = currentState;
        }

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
            if(controlMode == FingersMode) {
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
                    dq_hand[0] = dq_hand[1] = dq_hand[2] = 0.0;
                }
                qref[ioFinger1->jointId()] += radian(dq_hand[0]);
                qref[ioFinger2->jointId()] += radian(dq_hand[1]);
                qref[ioFinger3->jointId()] += radian(dq_hand[2]);
            } else {
                VectorXd p2(6);
                p2.head<3>() = ikWrist->p();
                p2.tail<3>() = rpyFromRot(ikWrist->R());

                VectorXd p3(6);
                if(controlMode == TranslationMode) {
                    p3.head<3>() = ikWrist->p() + Vector3(-pos[0], pos[1], pos[2]) * 0.2 * timeStep;
                    p3.tail<3>() = rpyFromRot(ikWrist->R());
                } else if(controlMode == WristMode) {
                    p3.head<3>() = ikWrist->p();
                    p3.tail<3>() = rpyFromRot(ikWrist->R() * rotFromRpy(Vector3(pos[0], -pos[1], -pos[2]) * 1.06 * timeStep));
                }

                wristInterpolator.clear();
                wristInterpolator.appendSample(time, p2);
                wristInterpolator.appendSample(time + timeStep, p3);
                wristInterpolator.update();
                phase = 2;
            }
        } else if(phase == 2) {
            if(time > wristInterpolator.domainUpper()) {
                phase = 0;
            }
        } else if(phase == 3) {
            qref = jointInterpolator.interpolate(time);
            if(time > jointInterpolator.domainUpper()) {
                for(int i = 0; i < ioBody->numJoints(); ++i) {
                    Link* joint = ioBody->joint(i);
                    double q = joint->q();
                    ikBody->joint(i)->q() = q;
                    qold[i] = q;
                }

                baseToWrist->calcForwardKinematics();

                VectorXd p2(6);
                p2.head<3>() = ikWrist->p();
                p2.tail<3>() = rpyFromRot(ikWrist->R());

                VectorXd p3(6);
                p3.head<3>() = ikWrist->p();
                p3.tail<3>() = rpyFromRot(ikWrist->R());

                wristInterpolator.clear();
                wristInterpolator.appendSample(time, p2);
                wristInterpolator.appendSample(time + timeStep, p3);
                wristInterpolator.update();
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

CNOID_IMPLEMENT_SIMPLE_CONTROLLER_FACTORY(Gen2JoystickController)
