/**
   \file
   \author Kenta Suzuki
*/

#include "SimulationManager.h"
#include <cnoid/Action>
#include <cnoid/AppConfig>
#include <cnoid/Joystick>
#include <cnoid/JoystickCapture>
#include <cnoid/MenuManager>
#include <cnoid/SimulationBar>
#include <cnoid/SimulatorItem>
#include "gettext.h"

using namespace cnoid;
using namespace std;

namespace {

SimulationManager* smanagerInstance = nullptr;
Action* startCheck = nullptr;

}

namespace cnoid {

class SimulationManagerImpl
{
public:
  SimulationManagerImpl(SimulationManager* self);
  virtual ~SimulationManagerImpl();
  SimulationManager* self;

  bool startState;
  bool pauseState;
  JoystickCapture joystick;
  SimulatorItem* simulatoritem;
  SimulationBar* sb;

  void onButton(const int& id, const bool& isPressed);
  void onSimulationAboutToStart(SimulatorItem* simulatorItem);
  void store(Mapping& archive);
  void restore(const Mapping& archive);
};

}


SimulationManager::SimulationManager()
{
    impl = new SimulationManagerImpl(this);
}


SimulationManagerImpl::SimulationManagerImpl(SimulationManager* self)
    : self(self),
      sb(SimulationBar::instance())
{
    startState = false;
    pauseState = false;
    joystick.setDevice("/dev/input/js0");
    joystick.sigButton().connect([&](int id, bool isPressed){ onButton(id, isPressed); });

    sb->sigSimulationAboutToStart().connect(
                [&](SimulatorItem* simulatorItem){ onSimulationAboutToStart(simulatorItem); });

    Mapping& config = *AppConfig::archive()->openMapping("simulation_manager");
    if(config.isValid()) {
        restore(config);
    }
}


SimulationManager::~SimulationManager()
{
    delete impl;
}


SimulationManagerImpl::~SimulationManagerImpl()
{
    store(*AppConfig::archive()->openMapping("simulation_manager"));
}


void SimulationManager::initializeClass(ExtensionManager* ext)
{
    MenuManager& mm = ext->menuManager().setPath("/" N_("Options")).setPath(N_("Joystick"));
    startCheck = mm.addCheckItem(_("Start simulation (StartButton)"));

    if(!smanagerInstance) {
        smanagerInstance = ext->manage(new SimulationManager);
    }
}


void SimulationManagerImpl::onButton(const int& id, const bool& isPressed)
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


void SimulationManagerImpl::onSimulationAboutToStart(SimulatorItem* simulatorItem)
{
    this->simulatoritem = simulatorItem;
    startState = true;
}


void SimulationManagerImpl::store(Mapping& archive)
{
    archive.write("use_start_button", startCheck->isChecked());
}


void SimulationManagerImpl::restore(const Mapping& archive)
{
    startCheck->setChecked(archive.get("use_start_button", false));
}