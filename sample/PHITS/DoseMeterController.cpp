/**
   Detector Controller
   @author Kenta Suzuki
*/

#include <cnoid/DoseMeter>
#include <cnoid/Joystick>
#include <cnoid/SimpleController>
#include <fmt/format.h>

using namespace std;
using namespace cnoid;

class DoseMeterController : public SimpleController
{
    Joystick joystick;
    std::ostream* os;
    DeviceList<DoseMeter> doseMeters;

public:
    virtual bool initialize(SimpleControllerIO* io) override
    {
        os = &io->os();
        Body* body = io->body();
        doseMeters = body->devices();

        for(size_t i = 0; i < doseMeters.size(); i++) {
            io->enableInput(doseMeters[i]);
        }

        return true;
    }

    virtual bool control() override
    {
        joystick.readCurrentState();

        for(size_t i = 0; i < doseMeters.size(); ++i) {
            DoseMeter* doseMeter = doseMeters[i];
            bool buttonStates[2] =  { false };
            for(int j = 0; j < 2; ++j) {
                buttonStates[j] = joystick.getButtonDown(
                            j == 0 ? Joystick::X_BUTTON : Joystick::Y_BUTTON);
            }

            if(buttonStates[0]) {
                doseMeter->setShield(!doseMeter->isShield());
                doseMeter->notifyStateChange();
            }

            if(buttonStates[1]) {
                (*os) << doseMeter->name();
                if(!doseMeter->isShield()) {
                    (*os) << ", NS, ";
                } else {
                    (*os) << ", WS, ";
                }
                (*os) << doseMeter->doseRate() << " [uSv/h], ";
                (*os) << doseMeter->integralDose() << " [uSv]" << endl;
            }
        }

        return true;
    }
};

CNOID_IMPLEMENT_SIMPLE_CONTROLLER_FACTORY(DoseMeterController)