/**
   Fire Event Controller
   @author Kenta Suzuki
*/

#include <cnoid/SimpleController>
#include <cnoid/FireDevice>
#include <cnoid/MathUtil>

using namespace cnoid;

class FireEventController : public SimpleController
{
    FireDevice* fire;
    double time;
    double timeStep;

public:
    virtual bool initialize(SimpleControllerIO* io) override
    {
        fire = io->body()->findDevice<FireDevice>("Fire");
        time = 0.0;
        timeStep = io->timeStep();
        return true;
    }

    virtual bool control() override
    {
        static const double T = 3.0;
        static const double w = radian(360.0) / T;
        if(sin(w * time) >= 1.0) {
            fire->on(!fire->on());
            fire->notifyStateChange();
        }

        time += timeStep;

        return true;
    }
};

CNOID_IMPLEMENT_SIMPLE_CONTROLLER_FACTORY(FireEventController)
