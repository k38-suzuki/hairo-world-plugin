/**
   \file
   \author Kenta Suzuki
*/

#include "KIOSKManager.h"
#include <cnoid/Action>
#include <cnoid/Archive>
#include <cnoid/ExecutablePath>
#include <cnoid/Joystick>
#include <cnoid/JoystickCapture>
#include <cnoid/MainWindow>
#include <cnoid/MenuManager>
#include <cnoid/OptionManager>
#include <cnoid/SimulationBar>
#include <cnoid/SimulatorItem>
#include <cnoid/TimeBar>
#include <cnoid/ViewArea>
#include <src/Base/ToolBarArea.h>
#include <QMenuBar>
#include <QMessageBox>
#include <QStatusBar>
#include "BookmarkManagerDialog.h"
#include "JoyKey.h"
#include "gettext.h"

using namespace cnoid;
using namespace std;

namespace {

Action* kioskCheck = nullptr;
Action* hideCheck = nullptr;

void onProjectOptionsParsed(boost::program_options::variables_map& v)
{
    bool result = false;
    char* CNOID_USE_KIOSK = getenv("CNOID_USE_KIOSK");
    if(v.count("kiosk")) {
        result = true;
    } else if(CNOID_USE_KIOSK && (strcmp(CNOID_USE_KIOSK, "0") == 0)) {
        result = true;
    }

    if(result) {
        kioskCheck->setChecked(false);
        kioskCheck->setChecked(true);
        BookmarkManagerDialog::instance()->show();
    }
}

void onEnableKIOSKToggled(const bool& on)
{
    MainWindow* mw = MainWindow::instance();
    mw->setFullScreen(on);
    mw->viewArea()->setViewTabsVisible(!on);
    mw->statusBar()->setVisible(!on);
    // onHideMenuBarToggled(on);
    // hideCheck->setChecked(on);
    if(on) {
        BookmarkManagerDialog::instance()->show();
    } else {
        BookmarkManagerDialog::instance()->hide();
    }
}

void onHideToolBarToggled(const bool& on)
{
    MainWindow::instance()->toolBarArea()->setVisible(!on);
}

}

namespace cnoid {

class KIOSKManagerImpl
{
public:
    KIOSKManagerImpl(KIOSKManager* self);
    KIOSKManager* self;

    JoystickCapture joystick;
    SimulatorItem* simulatorItem;
    JoyKey* key;

    void onHideMenuBarToggled(const bool& on);
    void onButton(const int& id, const bool& isPressed);
    void onSimulationAboutToStart(SimulatorItem* simulatorItem);
};

}


KIOSKManager::KIOSKManager(ExtensionManager* ext)
{
    impl = new KIOSKManagerImpl(this);
}


KIOSKManagerImpl::KIOSKManagerImpl(KIOSKManager* self)
    : self(self)
{
    joystick.setDevice("/dev/input/js0");
    joystick.sigButton().connect([&](int id, bool isPressed){ onButton(id, isPressed); });

    key = new JoyKey(true);
    key->sigUnlocked().connect([&](){ onHideMenuBarToggled(false); });

    simulatorItem = nullptr;
    SimulationBar::instance()->sigSimulationAboutToStart().connect(
                [&](SimulatorItem* simulatorItem){ onSimulationAboutToStart(simulatorItem); });
}


KIOSKManager::~KIOSKManager()
{
    delete impl;
}


void KIOSKManager::initializeClass(ExtensionManager* ext)
{
    MenuManager& mm = ext->menuManager().setPath("/" N_("View"));
    kioskCheck = mm.addCheckItem(_("Enable KIOSK"));
    kioskCheck->sigToggled().connect([&](bool on){ onEnableKIOSKToggled(on); });

    // hideCheck = mm.addCheckItem(_("Hide tool bar"));
    // hideCheck->sigToggled().connect([&](bool on){ onHideToolBarToggled(on); });

    OptionManager& om = ext->optionManager().addOption("kiosk", "start kiosk mode automatically");
    om.sigOptionsParsed(1).connect(
                [&](boost::program_options::variables_map& v){ onProjectOptionsParsed(v); });

    ext->manage(new KIOSKManager(ext));
    MainWindow::instance()->setFullScreen(false);
}


void KIOSKManagerImpl::onHideMenuBarToggled(const bool& on)
{
    MainWindow::instance()->menuBar()->setVisible(!on);
}


void KIOSKManagerImpl::onButton(const int& id, const bool& isPressed)
{
    if(id == Joystick::LOGO_BUTTON && isPressed) {
        if(kioskCheck->isChecked()) {
            if(simulatorItem) {
                if(simulatorItem->isRunning()) {
                    simulatorItem->stopSimulation(true);
                }
            }

            TimeBar* timeBar = TimeBar::instance();
            if(timeBar->isDoingPlayback()) {
                timeBar->stopPlayback(true);
            }
            timeBar->setTime(0.0);
            BookmarkManagerDialog::instance()->show();
        }
    }
}


void KIOSKManagerImpl::onSimulationAboutToStart(SimulatorItem* simulatorItem)
{
    this->simulatorItem = simulatorItem;
    if(kioskCheck->isChecked()) {
        BookmarkManagerDialog::instance()->hide();
    }
}
