/**
   TiltCamera Controller
   @author Kenta Suzuki
*/

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
        return true;
    }

    virtual bool control() override
    {
        joystick.readCurrentState();

        static const double P = 0.01;
        static const double D = 0.005;

        double  pos = -joystick.getPosition(Joystick::DIRECTIONAL_PAD_V_AXIS);
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

        return true;
    }
};

CNOID_IMPLEMENT_SIMPLE_CONTROLLER_FACTORY(TiltCameraJoystickController)
