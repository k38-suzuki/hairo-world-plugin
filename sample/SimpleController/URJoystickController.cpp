/**
   UR Controller
   @author Kenta Suzuki
*/

#include <cnoid/EigenUtil>
#include <cnoid/JointPath>
#include <cnoid/SharedJoystick>
#include <cnoid/SimpleController>
#include <sample/SimpleController/Interpolator.h>

using namespace std;
using namespace cnoid;

class URJoystickController : public SimpleController
{
    SimpleControllerIO* io;
    int jointActuationMode; 

    Body* ioBody;
    BodyPtr ikBody;
    Link* ikWrist;
    shared_ptr<JointPath> baseToWrist;
    VectorXd qref, qold, qref_old;
    Interpolator<VectorXd> wristInterpolator;
    int phase;
    double time;
    double timeStep;

    SharedJoystickPtr joystick;
    int targetMode;

public:

    virtual bool initialize(SimpleControllerIO* io) override
    {
        this->io = io;
        ostream& os = io->os();
        ioBody = io->body();

        jointActuationMode = Link::JointVelocity;
        for(auto opt : io->options()) {
            if(opt == "position") {
                jointActuationMode = Link::JointDisplacement;
                os << "The joint-position command mode is used." << endl;
            }
        }

        ikBody = ioBody->clone();
        const string& bodyName = ioBody->name();
        if(bodyName.find("Husky-UR5-2F85") != string::npos) {
            ikWrist = ikBody->link("UR5_Gripper_Base");
        } else {
            ikWrist = ikBody->link("Gripper_Base");
        }
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
        wristInterpolator.appendSample(0.001, p1);
        wristInterpolator.update();

        phase = 0;
        time = 0.0;
        timeStep = io->timeStep();

        joystick = io->getOrCreateSharedObject<SharedJoystick>("joystick");
        targetMode = joystick->addMode();

        return true;
    }

    virtual bool control() override
    {
        static const int axisID[] = {
            Joystick::L_STICK_V_AXIS, Joystick::L_STICK_H_AXIS, Joystick::DIRECTIONAL_PAD_V_AXIS,
            Joystick::DIRECTIONAL_PAD_H_AXIS, Joystick::R_STICK_H_AXIS, Joystick::R_STICK_V_AXIS
        };

        joystick->updateState(targetMode);

        double pos[6];
        for(int i = 0; i < 6; ++i) {
            pos[i] = joystick->getPosition(targetMode, axisID[i]);
            if(fabs(pos[i]) < 0.2) {
                pos[i] = 0.0;
            }
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
            if(joystick->mode() == targetMode) {
                VectorXd p0(6);
                p0.head<3>() = ikWrist->p();
                p0.tail<3>() = rpyFromRot(ikWrist->R());

                VectorXd p1(6);
                p1.head<3>() = ikWrist->p() + Vector3(-pos[0], -pos[1], -pos[2]) * 0.5 * timeStep;
                p1.tail<3>() = rpyFromRot(ikWrist->R() * rotFromRpy(Vector3(pos[3], pos[4], pos[5]) * 1.0 * timeStep));

                wristInterpolator.clear();
                wristInterpolator.appendSample(time + 0.0, p0);
                wristInterpolator.appendSample(time + timeStep, p1);
                wristInterpolator.update();
                phase = 2;
            }
        } else if(phase == 2) {
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

CNOID_IMPLEMENT_SIMPLE_CONTROLLER_FACTORY(URJoystickController)