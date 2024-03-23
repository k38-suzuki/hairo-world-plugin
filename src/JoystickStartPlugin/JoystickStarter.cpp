/**
   @author Kenta Suzuki
*/

#include "JoystickStarter.h"
#include <cnoid/Action>
#include <cnoid/AppConfig>
#include <cnoid/ExtensionManager>
#include <cnoid/Joystick>
#include <cnoid/JoystickCapture>
#include <cnoid/MenuManager>
#include <cnoid/SimulationBar>
#include <cnoid/SimulatorItem>
#include "gettext.h"

using namespace cnoid;
using namespace std;

namespace {

JoystickStarter* starterInstance = nullptr;
Action* startCheck = nullptr;

}

namespace cnoid {

class JoystickStarter::Impl
{
public:

  Impl();

  bool startState;
  bool pauseState;
  JoystickCapture joystick;
  SimulatorItem* simulatoritem;
  SimulationBar* sb;

  void onButton(const int& id, const bool& isPressed);
  void onSimulationAboutToStart(SimulatorItem* simulatorItem);
};

}


JoystickStarter::JoystickStarter()
{
    impl = new Impl;
}


JoystickStarter::Impl::Impl()
    : sb(SimulationBar::instance())
{
    startState = false;
    pauseState = false;
    joystick.setDevice("/dev/input/js0");
    joystick.sigButton().connect([&](int id, bool isPressed){ onButton(id, isPressed); });

    sb->sigSimulationAboutToStart().connect(
                [&](SimulatorItem* simulatorItem){ onSimulationAboutToStart(simulatorItem); });
}


JoystickStarter::~JoystickStarter()
{
    delete impl;
}


void JoystickStarter::initializeClass(ExtensionManager* ext)
{
    MenuManager& mm = ext->menuManager().setPath("/" N_("Options")).setPath(N_("Joystick"));
    startCheck = mm.addCheckItem(_("Game Start Mode"));
    startCheck->sigToggled().connect([&](bool on){
        AppConfig::archive()->openMapping("joystick_starter")->write("game_start_mode", startCheck->isChecked());
    });

    if(!starterInstance) {
        starterInstance = ext->manage(new JoystickStarter);
    }

    auto config = AppConfig::archive()->openMapping("joystick_starter");
    if(config->isValid()) {
        startCheck->setChecked(config->get("game_start_mode", false));
    }
}


void JoystickStarter::Impl::onButton(const int& id, const bool& isPressed)
{
    if(isPressed) {
        if(startCheck->isChecked()) {
            if(id == Joystick::START_BUTTON) {
                if(!startState) {
                    sb->startSimulation(true);
                } else {
                    if(simulatoritem) {
                        if(!pauseState) {
                            simulatoritem->pauseSimulation();
                        } else {
                            simulatoritem->restartSimulation();
                        }
                        pauseState = !pauseState;
                    }
                }
            } else if(id == Joystick::SELECT_BUTTON) {
                if(!startState) {
                    sb->startSimulation(false);
                } else {
                    if(simulatoritem) {
                        simulatoritem->stopSimulation(true);
                        startState = false;
                        pauseState = false;
                    }
                }
            }
        }
    }
}


void JoystickStarter::Impl::onSimulationAboutToStart(SimulatorItem* simulatorItem)
{
    this->simulatoritem = simulatorItem;
    startState = true;
}
