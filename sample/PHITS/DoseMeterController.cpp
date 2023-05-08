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
    bool prevButtonState[2];

public:
    virtual bool initialize(SimpleControllerIO* io) override
    {
        os = &io->os();
        Body* body = io->body();
        doseMeters = body->devices();
        prevButtonState[0] = prevButtonState[1] = false;

        for(size_t i = 0; i < doseMeters.size(); i++) {
            io->enableInput(doseMeters[i]);
        }

        return true;
    }

    virtual bool control() override
    {
        joystick.readCurrentState();

        for(int i = 0; i < 2; ++i) {
            bool buttonState = joystick.getButtonState(
                i == 0 ? Joystick::X_BUTTON : Joystick::Y_BUTTON);
            if(buttonState && !prevButtonState[i]) {
                for(int j = 0; j < doseMeters.size(); ++j) {
                    DoseMeter* doseMeter = doseMeters[j];
                    if(i == 0) {
                        doseMeter->setShield(!doseMeter->isShield());
                        doseMeter->notifyStateChange();
                    } else if(i == 1) {
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
            }
            prevButtonState[i] = buttonState;
        }

        return true;
    }
};

CNOID_IMPLEMENT_SIMPLE_CONTROLLER_FACTORY(DoseMeterController)
