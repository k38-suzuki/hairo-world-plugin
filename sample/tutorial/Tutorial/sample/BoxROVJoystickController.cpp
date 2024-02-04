/**
   BoxROV Controller
   @author Kenta Suzuki
*/

#include <cnoid/SimpleController>
#include <cnoid/Joystick>
#include <cnoid/Thruster>

using namespace std;
using namespace cnoid;

class BoxROVJoystickController : public SimpleController
{
    Thruster* thrusterL;
    Thruster* thrusterR;
    Joystick joystick;

public:

    virtual bool initialize(SimpleControllerIO* io) override
    {
        ostream& os = io->os();
        Body* body = io->body();
        thrusterL = body->findDevice<Thruster>("LeftThruster");
        thrusterR = body->findDevice<Thruster>("RightThruster");

        if(!thrusterL || !thrusterR) {
            os << "The thrusters are not found." << endl;
            return false;
        }

        return true;
    }

    virtual bool control() override
    {
        joystick.readCurrentState();

        double pos[2];
        for(int i = 0; i < 2; ++i) {
            pos[i] = joystick.getPosition(
                i == 0 ? Joystick::L_STICK_H_AXIS : Joystick::L_STICK_V_AXIS);
            if(fabs(pos[i]) < 0.2) {
                pos[i] = 0.0;
            }
        }

        double k = 0.05;
        thrusterL->force() = k * (-2.0 * pos[1] + pos[0]);
        thrusterR->force() = k * (-2.0 * pos[1] - pos[0]);
        thrusterL->notifyStateChange();
        thrusterR->notifyStateChange();

        return true;
    }
};

CNOID_IMPLEMENT_SIMPLE_CONTROLLER_FACTORY(BoxROVJoystickController)
