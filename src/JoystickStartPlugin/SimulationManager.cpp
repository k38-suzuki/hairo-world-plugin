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
#include <cnoid/ProjectManager>
#include <cnoid/SimulationBar>
#include <cnoid/SimulatorItem>
#include "gettext.h"

using namespace cnoid;
using namespace std;

namespace cnoid {

class SimulationManagerImpl
{
public:
  SimulationManagerImpl(SimulationManager* self, ExtensionManager* ext);
  virtual ~SimulationManagerImpl();
  SimulationManager* self;

  bool startState;
  bool pauseState;
  JoystickCapture joystick;
  Action* useStartButton;
  Action* useLogoButton;
  SimulatorItem* simulatoritem;
  SimulationBar* sb;

  void onButton(const int& id, const bool& isPressed);
  void store(Mapping& archive);
  void restore(const Mapping& archive);
};

}


SimulationManager::SimulationManager(ExtensionManager* ext)
{
    impl = new SimulationManagerImpl(this, ext);
}


SimulationManagerImpl::SimulationManagerImpl(SimulationManager* self, ExtensionManager* ext)
    : self(self),
      sb(SimulationBar::instance())
{
    MenuManager& mm = ext->menuManager().setPath("/Options").setPath(N_("Joystick"));
    useStartButton = mm.addCheckItem(_("Start simulation (StartButton)"));
//    useLogoButton = mm.addCheckItem(_("Open a project (LogoButton)"));
    useLogoButton = new Action;

    startState = false;
    pauseState = false;
    joystick.setDevice("/dev/input/js0");
    joystick.sigButton().connect([&](int id, bool isPressed){ onButton(id, isPressed); });

    sb->sigSimulationAboutToStart().connect(
                [&](SimulatorItem* simulatorItem){ this->simulatoritem = simulatorItem; });

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


void SimulationManager::initialize(ExtensionManager* ext)
{
    ext->manage(new SimulationManager(ext));
}


void SimulationManagerImpl::onButton(const int& id, const bool& isPressed)
{
    if(isPressed) {
        if(useStartButton->isChecked()) {
            if(id == Joystick::START_BUTTON) {
                if(!startState) {
                    sb->startSimulation(true);
                    startState = true;
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
                    startState = true;
                } else {
                    if(simulatoritem) {
                        simulatoritem->stopSimulation(true);
                        startState = false;
                        pauseState = false;
                    }
                }
            }
        }
        if(useLogoButton->isChecked()) {
            if(id == Joystick::LOGO_BUTTON) {
                ProjectManager::instance()->showDialogToLoadProject();
                startState = false;
                pauseState = false;
            }
        }
    }
}


void SimulationManagerImpl::store(Mapping& archive)
{
    archive.write("use_start_button", useStartButton->isChecked());
    archive.write("use_logo_button", useLogoButton->isChecked());
}


void SimulationManagerImpl::restore(const Mapping& archive)
{
    useStartButton->setChecked(archive.get("use_start_button", false));
    useLogoButton->setChecked(archive.get("use_logo_button", false));
}
