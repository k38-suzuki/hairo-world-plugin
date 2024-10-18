/**
   @author Kenta Suzuki
*/

#include "JoystickStarter.h"
#include <cnoid/Action>
#include <cnoid/AppConfig>
#include <cnoid/ExtensionManager>
#include <cnoid/Format>
#include <cnoid/Joystick>
#include <cnoid/JoystickCapture>
#include <cnoid/MainMenu>
#include <cnoid/MainWindow>
#include <cnoid/MenuManager>
#include <cnoid/SimulationBar>
#include <cnoid/SimulatorItem>
#include <cnoid/TimeBar>
#include <cnoid/ValueTree>
#include <QStatusBar>
#include "gettext.h"

using namespace std;
using namespace cnoid;

namespace {

bool is_starter_enabled = false;

}

namespace cnoid {

class JoystickStarter::Impl
{
public:

    Impl();

    enum StatusId { START, STOP, PAUSE };

    JoystickCapture joystick;
    SimulatorItem* simulatoritem;
    SimulationBar* sb;

    QStatusBar* statusBar;

    StatusId currentStatus;

    void onButton(int id, bool isPressed);
    void onSimulationAboutToStart(SimulatorItem* simulatorItem);
    void onPlaybackStopped(double time, bool isStoppedManually);
};

}


void JoystickStarter::initializeClass(ExtensionManager* ext)
{
    auto config = AppConfig::archive()->openMapping("joystick_starter");
    is_starter_enabled = config->get("game_start_mode", false);

    if(auto optionsMenu = MainMenu::instance()->get_Options_Menu()) {
        MenuManager& mm = ext->menuManager();
        mm.setCurrent(optionsMenu).setPath(N_("Joystick"));
        auto currentMenu = mm.currentMenu();
        auto startCheck = mm.addCheckItem(_("Game Start Mode"));
        currentMenu->sigAboutToShow().connect(
            [startCheck](){ startCheck->setChecked(is_starter_enabled); });

        startCheck->sigToggled().connect(
            [&](bool checked){
                is_starter_enabled = checked;
                AppConfig::archive()->openMapping("joystick_starter")->write("game_start_mode", checked);
            });
    }

    static JoystickStarter* starter = nullptr;

    if(!starter) {
        starter = ext->manage(new JoystickStarter);
    }
}


JoystickStarter::JoystickStarter()
{
    impl = new Impl;
}


JoystickStarter::Impl::Impl()
    : sb(SimulationBar::instance())
{
    currentStatus = STOP;
    statusBar = MainWindow::instance()->statusBar();

    joystick.setDevice("/dev/input/js0");
    joystick.sigButton().connect([&](int id, bool isPressed){ onButton(id, isPressed); });

    sb->sigSimulationAboutToStart().connect(
        [&](SimulatorItem* simulatorItem){ onSimulationAboutToStart(simulatorItem); });

    TimeBar::instance()->sigPlaybackStopped().connect(
        [&](double time, bool isStoppedManually){ onPlaybackStopped(time, isStoppedManually); });
}


JoystickStarter::~JoystickStarter()
{
    delete impl;
}


void JoystickStarter::Impl::onButton(int id, bool isPressed)
{
    if(isPressed) {
        if(is_starter_enabled) {
            switch(id) {
                case Joystick::START_BUTTON:
                    if(currentStatus == STOP) {
                        currentStatus = START;
                        sb->startSimulation(true);
                        statusBar->showMessage(_("Stop simulation: SELECT button, Pause simulation: START button"));
                    } else {
                        if(currentStatus == START) {
                            currentStatus = PAUSE;
                            simulatoritem->pauseSimulation();
                            statusBar->showMessage(_("Stop simulation: SELECT button, Restart simulation: START button"));
                        } else {
                            currentStatus = START;
                            simulatoritem->restartSimulation();
                            statusBar->showMessage(_("Stop simulation: SELECT button, Pause simulation: START button"));
                        }
                    }
                    break;
                case Joystick::SELECT_BUTTON:
                    if(currentStatus == STOP) {
                        currentStatus = START;
                        sb->startSimulation(false);
                        statusBar->showMessage(_("Stop simulation: SELECT button, Pause simulation: START button"));
                    } else {
                        currentStatus = STOP;
                        simulatoritem->stopSimulation(true);
                        statusBar->showMessage(_("Start simulation from the current state: SELECT button, Start simulation from the beginning: START button"));
                    }
                    break;
                default:
                    break;
            }
        }
    }
}


void JoystickStarter::Impl::onSimulationAboutToStart(SimulatorItem* simulatorItem)
{
    currentStatus = START;
    this->simulatoritem = simulatorItem;
}


void JoystickStarter::Impl::onPlaybackStopped(double time, bool isStoppedManually)
{
    if(currentStatus == START) {
        currentStatus = STOP;
    }
}