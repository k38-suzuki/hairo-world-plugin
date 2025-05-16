/**
   Camera Controller
   @author Kenta Suzuki
*/

#include <cnoid/Camera>
#include <cnoid/MathUtil>
#include <cnoid/SharedJoystick>
#include <cnoid/SimpleController>

using namespace cnoid;

class TiltCameraJoystickController : public SimpleController
{
    Link* turret;
    double qref;
    double qprev;
    double timeStep;
    Camera* camera;

    SharedJoystickPtr joystick;
    int targetMode;

public:
    virtual bool initialize(SimpleControllerIO* io) override
    {
        Body* body = io->body();
        camera = body->findDevice<Camera>("Camera");

        turret = body->link("TURRET_T");

        if(!turret) {
            return false;
        }
        qref = qprev = turret->q();
        turret->setActuationMode(Link::JointEffort);
        io->enableIO(turret);

        timeStep = io->timeStep();

        joystick = io->getOrCreateSharedObject<SharedJoystick>("joystick");
        targetMode = joystick->addMode();

        return true;
    }

    virtual bool control() override
    {
        joystick->updateState(targetMode);

        double pos[2];
        for(int i = 0; i < 2; ++i) {
            pos[i] =
                joystick->getPosition(targetMode, i == 0 ? Joystick::DIRECTIONAL_PAD_V_AXIS : Joystick::L_TRIGGER_AXIS);
            if(fabs(pos[i]) < 0.2) {
                pos[i] = 0.0;
            }
        }

        static const double P = 0.01;
        static const double D = 0.005;

        double q = turret->q();
        double dq = (q - qprev) / timeStep;
        double dqref = 0.0;
        double deltaq = 0.00025 * pos[0];
        qref += deltaq;
        dqref = deltaq / timeStep;
        turret->u() = P * (qref - q) + D * (dqref - dq);
        qprev = q;

        double pos2 = 0.0;
        bool changed = false;
        if(!joystick->getButtonState(targetMode, Joystick::L_BUTTON)) {
            pos2 = pos[1];
        } else {
            pos2 = -1.0;
        }

        double fov = camera->fieldOfView();
        fov += radian(1.0) * pos2 * 0.02;
        if((fov > radian(0.0)) && (fov < radian(90.0))) {
            camera->setFieldOfView(fov);
            changed = true;
        }
        if(changed) {
            camera->notifyStateChange();
        }

        return true;
    }
};

CNOID_IMPLEMENT_SIMPLE_CONTROLLER_FACTORY(TiltCameraJoystickController)