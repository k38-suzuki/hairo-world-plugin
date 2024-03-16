/**
   Light Event Controller
   @author Kenta Suzuki
*/

#include <cnoid/SimpleController>
#include <cnoid/SpotLight>
#include <cnoid/MathUtil>

using namespace cnoid;

class LightEventController : public SimpleController
{
    SpotLight* light;
    double time;
    double timeStep;

public:
    virtual bool initialize(SimpleControllerIO* io) override
    {
        light = io->body()->findDevice<SpotLight>("Light");
        time = 0.0;
        timeStep = io->timeStep();
        return true;
    }

    virtual bool control() override
    {
        static const double T = 3.0;
        static const double w = radian(360.0) / T;
        if(sin(w * time) >= 1.0) {
            light->on(!light->on());
            light->notifyStateChange();
        }

        time += timeStep;

        return true;
    }
};

CNOID_IMPLEMENT_SIMPLE_CONTROLLER_FACTORY(LightEventController)
