/**
   Smoke Event Controller
   @author Kenta Suzuki
*/

#include <cnoid/SimpleController>
#include <src/SceneEffectsPlugin/SmokeDevice.h>
#include <cnoid/MathUtil>

using namespace cnoid;

class SmokeEventController : public SimpleController
{
    SmokeDevice* smoke;
    double time;
    double timeStep;

public:
    virtual bool initialize(SimpleControllerIO* io) override
    {
        smoke = io->body()->findDevice<SmokeDevice>("Smoke");
        time = 0.0;
        timeStep = io->timeStep();
        return true;
    }

    virtual bool control() override
    {
        static const double T = 3.0;
        static const double w = radian(360.0) / T;
        if(sin(w * time) >= 1.0) {
            smoke->on(!smoke->on());
            smoke->notifyStateChange();
        }

        time += timeStep;

        return true;
    }
};

CNOID_IMPLEMENT_SIMPLE_CONTROLLER_FACTORY(SmokeEventController)
