/**
   @author Kenta Suzuki
*/

#include "KIOSKManager.h"
#include <cnoid/AppConfig>
#include <cnoid/ExtensionManager>
#include <cnoid/Joystick>
#include <cnoid/JoystickCapture>
#include <cnoid/MainMenu>
#include <cnoid/MainWindow>
#include <cnoid/MenuManager>
#include <cnoid/OptionManager>
#include <cnoid/SimulationBar>
#include <cnoid/SimulatorItem>
#include <cnoid/TimeBar>
#include <cnoid/ViewArea>
#include <src/Base/ToolBarArea.h>
#include <QMenuBar>
#include <QStatusBar>
#include "BookmarkManager.h"
#include "JoyKey.h"
#include "gettext.h"

using namespace std;
using namespace cnoid;

namespace {

KIOSKManager* kioskInstance = nullptr;
bool is_kiosk_enabled = false;

void onKIOSKOptionsParsed(OptionManager*)
{
    if(is_kiosk_enabled) {
        kioskInstance->setKIOSKEnabled(true);
    }
}

}

namespace cnoid {

class KIOSKManager::Impl
{
public:

    Impl();
    ~Impl();

    void onKIOSKToggled(bool checked);
    void onHideMenuBarToggled(bool checked);
    void onHideToolBarToggled(bool checked);
    void onButton(const int& id, bool isPressed);

    JoystickCapture joystick;
    SimulatorItem* simulatorItem;
};

}


void KIOSKManager::initializeClass(ExtensionManager* ext)
{
    auto config = AppConfig::archive()->openMapping("kiosk_manager");
    is_kiosk_enabled = config->get("kiosk_mode", false);

    if(auto optionsMenu = MainMenu::instance()->get_Options_Menu()) {
        MenuManager& mm = ext->menuManager();
        mm.setCurrent(optionsMenu).setPath(N_("KIOSK"));
        auto currentMenu = mm.currentMenu();
        auto kioskCheck = mm.addCheckItem(_("Full screen Mode"));
        currentMenu->sigAboutToShow().connect(
            [kioskCheck](){ kioskCheck->setChecked(is_kiosk_enabled); });

        kioskCheck->sigToggled().connect(
            [&](bool checked){
                is_kiosk_enabled = checked;
                AppConfig::archive()->openMapping("kiosk_manager")->write("kiosk_mode", checked);
                kioskInstance->setKIOSKEnabled(checked);
            });
    }

    auto om = OptionManager::instance();
    om->add_flag("--kiosk-mode", is_kiosk_enabled, "start kiosk mode automatically");
    om->sigOptionsParsed(1).connect(onKIOSKOptionsParsed);

    char* CNOID_USE_KIOSK = getenv("CNOID_USE_KIOSK");
    if(CNOID_USE_KIOSK && (strcmp(CNOID_USE_KIOSK, "0") == 0)) {
        is_kiosk_enabled = true;
        kioskInstance->setKIOSKEnabled(true);
    }

    if(!kioskInstance) {
        kioskInstance = ext->manage(new KIOSKManager);
        MainWindow::instance()->setFullScreen(false);
    }
}


KIOSKManager* KIOSKManager::instance()
{
    return kioskInstance;
}


KIOSKManager::KIOSKManager()
{
    impl = new Impl;
}


KIOSKManager::Impl::Impl()
{
    joystick.setDevice("/dev/input/js0");
    joystick.sigButton().connect([&](int id, bool isPressed){ onButton(id, isPressed); });

    auto key = new JoyKey(true);
    key->sigUnlocked().connect([&](){ onHideMenuBarToggled(false); });

    simulatorItem = nullptr;
    SimulationBar::instance()->sigSimulationAboutToStart().connect(
                [&](SimulatorItem* simulatorItem){
                    this->simulatorItem = simulatorItem;
                    if(is_kiosk_enabled) {
                        BookmarkManager::instance()->hide();
                    }
                });
}


KIOSKManager::~KIOSKManager()
{
    delete impl;
}


KIOSKManager::Impl::~Impl()
{

}


void KIOSKManager::setKIOSKEnabled(bool checked)
{
    impl->onKIOSKToggled(checked);
}


void KIOSKManager::Impl::onKIOSKToggled(bool checked)
{
    MainWindow* mw = MainWindow::instance();
    mw->setFullScreen(checked);
    mw->viewArea()->setViewTabsVisible(!checked);
    mw->statusBar()->setVisible(!checked);
    // onHideMenuBarToggled(checked);
    onHideToolBarToggled(checked);
    checked ? BookmarkManager::instance()->show() : BookmarkManager::instance()->hide();
}


void KIOSKManager::Impl::onHideMenuBarToggled(bool checked)
{
    MainWindow::instance()->menuBar()->setVisible(!checked);
}


void KIOSKManager::Impl::onHideToolBarToggled(bool checked)
{
    MainWindow::instance()->toolBarArea()->setVisible(!checked);
}


void KIOSKManager::Impl::onButton(const int& id, bool isPressed)
{
    if(id == Joystick::LOGO_BUTTON && isPressed) {
        if(is_kiosk_enabled) {
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
            BookmarkManager::instance()->show();
        }
    }
}