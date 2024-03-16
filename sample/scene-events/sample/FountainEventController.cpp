/**
   Fountain Event Controller
   @author Kenta Suzuki
*/

#include <cnoid/SimpleController>
#include <cnoid/FountainDevice>
#include <cnoid/MathUtil>

using namespace cnoid;

class FountainEventController : public SimpleController
{
    FountainDevice* fountain;
    double time;
    double timeStep;

public:
    virtual bool initialize(SimpleControllerIO* io) override
    {
        fountain = io->body()->findDevice<FountainDevice>("Fountain");
        time = 0.0;
        timeStep = io->timeStep();
        return true;
    }

    virtual bool control() override
    {
        static const double T = 3.0;
        static const double w = radian(360.0) / T;
        if(sin(w * time) >= 1.0) {
            fountain->on(!fountain->on());
            fountain->notifyStateChange();
        }

        time += timeStep;

        return true;
    }
};

CNOID_IMPLEMENT_SIMPLE_CONTROLLER_FACTORY(FountainEventController)
