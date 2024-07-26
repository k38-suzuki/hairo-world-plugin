/**
   @author Kenta Suzuki
*/

#include "JoystickStarter.h"
#include <cnoid/Action>
#include <cnoid/AppConfig>
#include <cnoid/ExtensionManager>
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
#include <fmt/format.h>
#include "gettext.h"

using namespace std;
using namespace cnoid;
using fmt::format;

namespace {

JoystickStarter* starterInstance = nullptr;
Action* startCheck = nullptr;

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


void JoystickStarter::initializeClass(ExtensionManager* ext)
{
    if(auto optionsMenu = MainMenu::instance()->get_Options_Menu()) {
        MenuManager& mm = ext->menuManager();
        mm.setCurrent(optionsMenu).setPath(N_("Joystick"));
        startCheck = mm.addCheckItem(_("Game Start Mode"));
        startCheck->sigToggled().connect([&](bool on){
            auto& archive = *AppConfig::archive()->openMapping("joystick_starter");
            archive.write("game_start_mode", startCheck->isChecked()); });
    }

    if(!starterInstance) {
        starterInstance = ext->manage(new JoystickStarter);
    }

    auto& archive = *AppConfig::archive()->openMapping("joystick_starter");
    if(archive.isValid()) {
        startCheck->setChecked(archive.get("game_start_mode", false));
    }
}


void JoystickStarter::Impl::onButton(int id, bool isPressed)
{
    if(isPressed) {
        if(startCheck->isChecked()) {
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