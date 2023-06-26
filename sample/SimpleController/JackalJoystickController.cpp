/**
   Jackal Controller
   @author Kenta Suzuki
*/

#include <cnoid/SharedJoystick>
#include <cnoid/SimpleController>

using namespace std;
using namespace cnoid;

class JackalJoystickController : public SimpleController
{
    Link* wheel[4];

    SharedJoystickPtr joystick;
    int targetMode;

public:

    virtual bool initialize(SimpleControllerIO* io) override
    {
        ostream& os = io->os();
        Body* ioBody = io->body();

        static const char* linknames[] = {
            "FRONT_LEFT", "FRONT_RIGHT", "REAR_LEFT", "REAR_RIGHT"
        };

        for(int i = 0; i < 4; ++i) {
            wheel[i] = ioBody->link(linknames[i]);
            Link* joint = wheel[i];
            if(!joint) {
                os << "Wheel " << i << " is not found." << endl;
                return false;
            }
            joint->setActuationMode(Link::JointVelocity);
            io->enableIO(joint);
        }

        joystick = io->getOrCreateSharedObject<SharedJoystick>("joystick");
        targetMode = joystick->addMode();

        return true;
    }

    virtual bool control() override
    {
        joystick->updateState(targetMode);
        
        double pos[2];
        for(int i = 0; i < 2; ++i) {
            pos[i] = joystick->getPosition(targetMode,
                i == 0 ? Joystick::L_STICK_H_AXIS : Joystick::L_STICK_V_AXIS);
            if(fabs(pos[i]) < 0.2) {
                pos[i] = 0.0;
            }
        }

        double k = 0.0;
        for(int i = 0; i < 2; ++i) {
            bool buttonState = joystick->getButtonState(targetMode,
                i == 0 ? Joystick::L_BUTTON : Joystick::R_BUTTON);
            if(buttonState) {
                if(i == 0) {
                    k = 0.2 / 0.098 / 2.0;
                } else if(i == 1) {
                    k = 1.0 / 0.098 / 2.0;
                }
            }
        }

        for(int i = 0; i < 2; ++i) {
            Link* wheelL = wheel[2 * i];
            Link* wheelR = wheel[2 * i + 1];
            wheelL->dq_target() = k * (-2.0 * pos[1] + pos[0]);
            wheelR->dq_target() = k * (-2.0 * pos[1] - pos[0]);
        }

        return true;
    }
};

CNOID_IMPLEMENT_SIMPLE_CONTROLLER_FACTORY(JackalJoystickController)
