#include <cnoid/Joystick>
#include <cnoid/SimpleController>

using namespace cnoid;

class WingController : public SimpleController
{ 
    Link* joints[2];
    Joystick joystick;

public:
    virtual bool initialize(SimpleControllerIO* io) override
    {
        joints[0] = io->body()->link("WING_L");
        joints[1] = io->body()->link("WING_R");

        for(int i = 0; i < 2; ++i) {
            Link* joint = joints[i];
            joint->setActuationMode(Link::JointVelocity);
            io->enableIO(joint);
        }
        
        return true;
    }

    virtual bool control() override
    {
        static const int axisID[] = { 1, 1 };

        joystick.readCurrentState();

        double pos[2];
        for(int i = 0; i < 2; ++i) {
            Link* joint = joints[i];
            pos[i] = joystick.getPosition(axisID[i]);
            if(fabs(pos[i]) < 0.2) {
                pos[i] = 0.0;
            }
            
            joint->dq_target() = -0.5 * pos[i];
        }

        return true;
    }
};

CNOID_IMPLEMENT_SIMPLE_CONTROLLER_FACTORY(WingController)