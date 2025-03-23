/**
    UGV Controller
    @author Kenta Suzuki
*/

#include <cnoid/SimpleController>
#include <cnoid/SharedJoystick>
#include <vector>

using namespace std;
using namespace cnoid;

class UGVJoystickController : public SimpleController
{
    Link* wheel[4];
    double radius;

    struct UGVInfo {
        char* bodyName;
        double radius;
        UGVInfo(char* bodyName, double radius)
            : bodyName(bodyName),
              radius(radius)
        { }
    };
    vector<UGVInfo> ugvs;

    SharedJoystickPtr joystick;
    int targetMode;

public:

    virtual bool initialize(SimpleControllerIO* io) override
    {
        ostream& os = io->os();

        Body* body = io->body();

        wheel[0] = body->link("FRONT_LEFT");
        wheel[1] = body->link("FRONT_RIGHT");
        wheel[2] = body->link("REAR_LEFT");
        wheel[3] = body->link("REAR_RIGHT");
        for(int i = 0; i < 4; ++i) {
            if(!wheel[i]) {
                os << "The wheel links are not found." << endl;
                return false;
            }
            wheel[i]->setActuationMode(Link::JointTorque);
            io->enableInput(wheel[i], JointVelocity);
            io->enableOutput(wheel[i]);
        }

        ugvs = {
            { "Jackal"        , 0.0980 },
            { "Husky"         , 0.1651 },
            { "Husky-UR5-2F85", 0.1651 }
        };

        for(auto& info : ugvs) {
            if(body->name() == info.bodyName) {
                radius = info.radius;
                os << info.bodyName << " is found." << endl;
            }
        }

        joystick = io->getOrCreateSharedObject<SharedJoystick>("joystick");
        targetMode = joystick->addMode();

        return true;
    }

    virtual bool control() override
    {
        joystick->updateState(targetMode);

        static const double DRIVE_GAIN = 1.0;
        static const double VEL_MAX = 1.0 / radius / 2.0;
        static const double VEL_MIN = 0.2 / radius / 2.0;

        bool is_lb_pressed = false;
        bool is_rb_pressed = false;
        for(int i = 0; i < 2; ++i) {
            bool buttonState = joystick->getButtonState(
                i == 0 ? Joystick::L_BUTTON : Joystick::R_BUTTON);
            if(buttonState) {
                if(i == 0) {
                    is_lb_pressed = true;
                } else if(i == 1) {
                    is_rb_pressed = true;
                }
            }
        }

        double pos[2];
        pos[0] = joystick->getPosition(targetMode, 0);
        pos[1] = joystick->getPosition(targetMode, 1);

        for(int i = 0; i < 4; ++i) {
            double dq = wheel[i]->dq();
            double drive_ref = (-2.0 * pos[1] + (i % 2 == 0 ? pos[0] : -pos[0])) * (is_rb_pressed ? VEL_MAX : (is_lb_pressed ? VEL_MIN : 0.0));
            wheel[i]->u() = (drive_ref - dq) * DRIVE_GAIN;
        }

        return true;
    }
};

CNOID_IMPLEMENT_SIMPLE_CONTROLLER_FACTORY(UGVJoystickController)