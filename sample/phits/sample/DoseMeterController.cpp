/**
   Detector Controller
   @author Kenta Suzuki
*/

#include <cnoid/DoseMeter>
#include <cnoid/Joystick>
#include <cnoid/SimpleController>
#include <vector>

using namespace std;
using namespace cnoid;

class DoseMeterController : public SimpleController
{
    DeviceList<DoseMeter> doseMeters;
    Joystick joystick;
    std::ostream* os;

    struct ActionInfo {
        int actionId;
        int buttonId;
        bool prevButtonState;
        bool stateChanged;
        ActionInfo(int actionId, int buttonId)
            : actionId(actionId),
              buttonId(buttonId),
              prevButtonState(false),
              stateChanged(false)
        { }
    };
    vector<ActionInfo> actions;

public:
    virtual bool initialize(SimpleControllerIO* io) override
    {
        os = &io->os();
        Body* body = io->body();
        doseMeters = body->devices();

        for(auto& doseMeter : doseMeters) {
            io->enableInput(doseMeter);
        }

        actions = {
            { 0, Joystick::X_BUTTON },
            { 1, Joystick::Y_BUTTON }
        };

        return true;
    }

    virtual bool control() override
    {
        joystick.readCurrentState();

        for(auto& info : actions) {
            bool stateChanged = false;
            bool buttonState = joystick.getButtonState(info.buttonId);
            if(buttonState && !info.prevButtonState) {
                stateChanged = true;
            }
            info.prevButtonState = buttonState;
            if(stateChanged) {
                for(int j = 0; j < doseMeters.size(); ++j) {
                    DoseMeter* doseMeter = doseMeters[j];
                    if(info.actionId == 0) {
                        doseMeter->setShield(!doseMeter->isShield());
                        doseMeter->notifyStateChange();
                    } else if(info.actionId == 1) {
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
        }

        return true;
    }
};

CNOID_IMPLEMENT_SIMPLE_CONTROLLER_FACTORY(DoseMeterController)
