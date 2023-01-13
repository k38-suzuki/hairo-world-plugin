/**
   \file
   \author Kenta Suzuki
*/

#include "KIOSKManager.h"
#include <cnoid/Action>
#include <cnoid/AppConfig>
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
#include <cnoid/BookmarkManager>
#include <QMenuBar>
#include <QMessageBox>
#include <QStatusBar>
#include "JoyKey.h"
#include "gettext.h"

using namespace cnoid;
using namespace std;

namespace {

Action* enable_kiosk = nullptr;

}

namespace cnoid {

class KIOSKManagerImpl
{
public:
    KIOSKManagerImpl(ExtensionManager* ext, KIOSKManager* self);
    KIOSKManager* self;
    virtual ~KIOSKManagerImpl();

    Action* hide_toolBar;
    JoystickCapture joystick;
    SimulatorItem* simulatorItem;
    JoyKey* key;

    void onProjectOptionsParsed(boost::program_options::variables_map& v);
    void onEnableKIOSKToggled(const bool& on);
    void onHideMenuBarToggled(const bool& on);
    void onHideToolBarToggled(const bool& on);
    void onButton(const int& id, const bool& isPressed);
    void store(Mapping& archive);
    void restore(const Mapping& archive);
};

}


KIOSKManager::KIOSKManager(ExtensionManager* ext)
{
    impl = new KIOSKManagerImpl(ext, this);
}


KIOSKManagerImpl::KIOSKManagerImpl(ExtensionManager* ext, KIOSKManager* self)
    : self(self)
{
    MenuManager& mm = ext->menuManager();
    mm.setPath("/" N_("View"));
    enable_kiosk = mm.addCheckItem(_("Enable KIOSK"));
    enable_kiosk->sigToggled().connect([&](bool on){ onEnableKIOSKToggled(on); });

    mm.setPath("/" N_("View"));
    hide_toolBar = mm.addCheckItem(_("Hide tool bar"));
    hide_toolBar->sigToggled().connect([&](bool on){ onHideToolBarToggled(on); });

    joystick.setDevice("/dev/input/js0");
    joystick.sigButton().connect([&](int id, bool isPressed){ onButton(id, isPressed); });

    key = new JoyKey(true);
    key->sigUnlocked().connect([&](){ onHideMenuBarToggled(false); });

    OptionManager& om = ext->optionManager().addOption("kiosk", "start kiosk mode automatically");
    om.sigOptionsParsed(1).connect(
                [&](boost::program_options::variables_map& v){ onProjectOptionsParsed(v); });

    simulatorItem = nullptr;
    SimulationBar* sb = SimulationBar::instance();
    sb->sigSimulationAboutToStart().connect(
                [&](SimulatorItem* simulatorItem){ this->simulatorItem = simulatorItem; });

    Mapping& config = *AppConfig::archive()->openMapping("kiosk");
    if(config.isValid()) {
        restore(config);
    }
}


KIOSKManager::~KIOSKManager()
{
    delete impl;
}


KIOSKManagerImpl::~KIOSKManagerImpl()
{
    store(*AppConfig::archive()->openMapping("kiosk"));
}


void KIOSKManager::initializeClass(ExtensionManager* ext)
{
    ext->manage(new KIOSKManager(ext));
    MainWindow::instance()->setFullScreen(false);
}


void KIOSKManagerImpl::onProjectOptionsParsed(boost::program_options::variables_map& v)
{
    bool result = false;
    char* CNOID_USE_KIOSK = getenv("CNOID_USE_KIOSK");
    if(v.count("kiosk")) {
        result = true;
    } else if(CNOID_USE_KIOSK && (strcmp(CNOID_USE_KIOSK, "0") == 0)) {
        result = true;
    }

    if(result) {
        enable_kiosk->setChecked(false);
        enable_kiosk->setChecked(true);
    }
}


void KIOSKManagerImpl::onEnableKIOSKToggled(const bool& on)
{
    MainWindow* mw = MainWindow::instance();
    mw->setFullScreen(on);
    mw->viewArea()->setViewTabsVisible(!on);
    mw->statusBar()->setVisible(!on);
    // onHideMenuBarToggled(on);
    hide_toolBar->setChecked(on);
}


void KIOSKManagerImpl::onHideMenuBarToggled(const bool& on)
{
    MainWindow::instance()->menuBar()->setVisible(!on);
}


void KIOSKManagerImpl::onHideToolBarToggled(const bool& on)
{
    MainWindow::instance()->toolBarArea()->setVisible(!on);
}


void KIOSKManagerImpl::onButton(const int& id, const bool& isPressed)
{
    if(id == Joystick::LOGO_BUTTON && isPressed) {
        if(enable_kiosk->isChecked()) {
            int ret = QMessageBox::question(MainWindow::instance(),
                                            _("KIOSK"), _("Would you like to return to the home screen?"));
            if(ret == QMessageBox::Yes) {
                if(simulatorItem) {
                    if(simulatorItem->isRunning()) {
                        simulatorItem->stopSimulation(true);
                    }
                }

                TimeBar* tb = TimeBar::instance();
                if(tb->isDoingPlayback()) {
                    tb->stopPlayback(true);
                }
                tb->setTime(0.0);
                // BookmarkManager::instance()->showBookmarkManagerDialog();
            } else {
                onHideMenuBarToggled(false);
            }
        }
    }
}


void KIOSKManagerImpl::store(Mapping& archive)
{

}


void KIOSKManagerImpl::restore(const Mapping& archive)
{

}