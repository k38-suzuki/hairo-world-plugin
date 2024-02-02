/**
   Camera Controller
   @author Kenta Suzuki
*/

#include <cnoid/Camera>
#include <cnoid/EigenUtil>
#include <cnoid/Joystick>
#include <cnoid/SimpleController>

using namespace cnoid;

class TiltCameraJoystickController : public SimpleController
{
    SimpleControllerIO* io;
    Link* turret;
    double qref;
    double qprev;
    double dt;
    Camera* camera;

    Joystick joystick;

public:

    virtual bool initialize(SimpleControllerIO* io) override
    {
        Body* body = io->body();
        turret = body->link("TURRET_T");
        if(turret) {
            turret->setActuationMode(Link::JointEffort);
            qref = qprev = turret->q();
            io->enableIO(turret);
        }

        dt = io->timeStep();
        camera = body->findDevice<Camera>("Camera");

        return true;
    }

    virtual bool control() override
    {
        joystick.readCurrentState();

        static const double P = 0.01;
        static const double D = 0.005;

        double  pos = joystick.getPosition(Joystick::DIRECTIONAL_PAD_V_AXIS) * -1.0;
        if(fabs(pos) < 0.15) {
            pos = 0.0;
        }

        double q = turret->q();
        double dq = (q - qprev) / dt;
        double dqref = 0.0;
        double deltaq = 0.00025 * pos;
        qref += deltaq;
        dqref = deltaq / dt;
        turret->u() = P * (qref - q) + D * (dqref - dq);
        qprev = q;

        double p = 0.0;
        bool changed = false;
        bool buttonState = joystick.getButtonState(Joystick::L_BUTTON);
        if(!buttonState) {
            p = joystick.getPosition(Joystick::L_TRIGGER_AXIS);
        } else  {
            p = -1.0;
        }

        if(fabs(p) < 0.15) {
            p = 0.0;
        }

        if(camera && fabs(p) > 0.0) {
            double fov = camera->fieldOfView();
            fov += radian(1.0) * p * 0.02;
            if((fov > radian(0.0)) && (fov < radian(90.0))) {
                camera->setFieldOfView(fov);
                changed = true;
            }
            if(changed) {
                camera->notifyStateChange();
            }
        }

        return true;
    }
};

CNOID_IMPLEMENT_SIMPLE_CONTROLLER_FACTORY(TiltCameraJoystickController)