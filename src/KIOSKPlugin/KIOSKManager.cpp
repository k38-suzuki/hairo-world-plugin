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
#include <cnoid/MessageView>
#include <cnoid/ProjectManager>
#include <cnoid/SimulationBar>
#include <cnoid/SimulatorItem>
#include <cnoid/TimeBar>
#include <cnoid/UTF8>
#include <cnoid/ViewArea>
#include <cnoid/WorldItem>
#include <src/BodyPlugin/WorldLogFileItem.h>
#include <QDateTime>
#include <QDir>
#include <QMenuBar>
#include <QMessageBox>
#include <QStatusBar>
#include "src/Base/ToolBarArea.h"
#include "KIOSKView.h"
#include "gettext.h"

using namespace cnoid;
using namespace std;

namespace {

struct Command {
    int id;
    double position;
    bool isPressed;
};

Command konamiCommand[] = {
    { Joystick::DIRECTIONAL_PAD_V_AXIS, -1.0, false },
    { Joystick::DIRECTIONAL_PAD_V_AXIS,  0.0, false },
    { Joystick::DIRECTIONAL_PAD_V_AXIS, -1.0, false },
    { Joystick::DIRECTIONAL_PAD_V_AXIS,  0.0, false },
    { Joystick::DIRECTIONAL_PAD_V_AXIS,  1.0, false },
    { Joystick::DIRECTIONAL_PAD_V_AXIS,  0.0, false },
    { Joystick::DIRECTIONAL_PAD_V_AXIS,  1.0, false },
    { Joystick::DIRECTIONAL_PAD_V_AXIS,  0.0, false },

    { Joystick::DIRECTIONAL_PAD_H_AXIS, -1.0, false },
    { Joystick::DIRECTIONAL_PAD_H_AXIS,  0.0, false },
    { Joystick::DIRECTIONAL_PAD_H_AXIS,  1.0, false },
    { Joystick::DIRECTIONAL_PAD_H_AXIS,  0.0, false },
    { Joystick::DIRECTIONAL_PAD_H_AXIS, -1.0, false },
    { Joystick::DIRECTIONAL_PAD_H_AXIS,  0.0, false },
    { Joystick::DIRECTIONAL_PAD_H_AXIS,  1.0, false },
    { Joystick::DIRECTIONAL_PAD_H_AXIS,  0.0, false },

    {               Joystick::A_BUTTON,  1.0,  true },
    {               Joystick::A_BUTTON,  1.0, false },
    {               Joystick::B_BUTTON,  1.0,  true },
    {               Joystick::B_BUTTON,  1.0, false },
};

Action* enable_kiosk = nullptr;

}


namespace cnoid {

class KIOSKManagerImpl
{
public:
    KIOSKManagerImpl(ExtensionManager* ext, KIOSKManager* self);
    KIOSKManager* self;
    virtual ~KIOSKManagerImpl();

    Action* hide_menuBar;
    Action* hide_toolBar;
    QDateTime recordingStartTime;
    bool isInitialized;
    JoystickCapture joystick;
    int password;
    SimulatorItem* simulatorItem;

    void loadProject(const bool& enabled);
    void onEnableKIOSKToggled(const bool& on);
    void onHideMenuBarToggled(const bool& on);
    void onHideToolBarToggled(const bool& on);
    void onProjectLoaded(const int& recursiveLevel);
    void onSimulationAboutToStart(SimulatorItem* simulatorItem);
    void onAxis(const int& id, const double& position);
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
    mm.setPath("/Options");
    mm.setPath("KIOSK");
    enable_kiosk = mm.addCheckItem(_("Enable KIOSK"));
    enable_kiosk->sigToggled().connect([&](bool on){ onEnableKIOSKToggled(on); });
    hide_menuBar = mm.addCheckItem(_("Hide menu bar"));
    hide_menuBar->setEnabled(false);
    hide_menuBar->sigToggled().connect([&](bool on){ onHideMenuBarToggled(on); });
    hide_toolBar = mm.addCheckItem(_("Hide tool bar"));
    hide_toolBar->sigToggled().connect([&](bool on){ onHideToolBarToggled(on); });
    isInitialized = false;
    joystick.setDevice("/dev/input/js0");
    joystick.sigAxis().connect([&](int id, double position){ onAxis(id, position); });
    joystick.sigButton().connect([&](int id, bool isPressed){ onButton(id, isPressed); });
    password = 0;
    simulatorItem = nullptr;

    ProjectManager* pm = ProjectManager::instance();
    pm->sigProjectLoaded().connect([&](int recursiveLevel){ onProjectLoaded(recursiveLevel); });
    SimulationBar* sb = SimulationBar::instance();
    sb->sigSimulationAboutToStart().connect(
                [&](SimulatorItem* simulatorItem){ onSimulationAboutToStart(simulatorItem); });

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


void KIOSKManager::initialize(ExtensionManager* ext)
{
    ext->manage(new KIOSKManager(ext));

    char* CNOID_USE_KIOSK = getenv("CNOID_USE_KIOSK");
    if(CNOID_USE_KIOSK && (strcmp(CNOID_USE_KIOSK, "0") == 0)) {
        enable_kiosk->setChecked(true);
    }
}


void KIOSKManagerImpl::loadProject(const bool& enabled)
{
    ProjectManager* pm = ProjectManager::instance();
    string filename;
    if(enabled) {
        filename = toUTF8((shareDirPath() / "kiosk" / "layout_kiosk.cnoid").string());
    } else {
        filename = toUTF8((shareDirPath() / "kiosk" / "layout_base.cnoid").string());
    }
    pm->clearProject();
    MessageView::instance()->flush();
    pm->loadProject(filename);
    pm->clearProject();
}


void KIOSKManagerImpl::onEnableKIOSKToggled(const bool& on)
{
    MainWindow* mw = MainWindow::instance();
    mw->setFullScreen(on);
    mw->viewArea()->setViewTabsVisible(!on);
    mw->statusBar()->setVisible(!on);
    hide_menuBar->setEnabled(on);
    hide_toolBar->setChecked(on);
    TimeBar* tb = TimeBar::instance();
    if(tb->isDoingPlayback()) {
        tb->stopPlayback(true);
        tb->setTime(0.0);
    }

    if(isInitialized) {
        loadProject(on);
    }
}


void KIOSKManagerImpl::onHideMenuBarToggled(const bool& on)
{
    MainWindow* mw = MainWindow::instance();
    mw->menuBar()->setVisible(!on);
}


void KIOSKManagerImpl::onHideToolBarToggled(const bool& on)
{
    MainWindow* mw = MainWindow::instance();
    mw->toolBarArea()->setVisible(!on);
}


void KIOSKManagerImpl::onProjectLoaded(const int& recursiveLevel)
{
    isInitialized = true;
}


void KIOSKManagerImpl::onSimulationAboutToStart(SimulatorItem* simulatorItem)
{
    this->simulatorItem = simulatorItem;
    string directory = toUTF8((shareDirPath() / "kiosk").string());
    QDir dir(directory.c_str());
    if(!dir.exists("logs")) {
        dir.mkdir("logs");
    }

    recordingStartTime = QDateTime::currentDateTime();
    string suffix = recordingStartTime.toString("yyyy-MM-dd-hh-mm-ss").toStdString();
    string filename = directory + "/logs/" + suffix;
    string projectFile = filename + ".cnoid";

    bool isLoggingEnabled = false;
    KIOSKView* kioskView = KIOSKView::instance();
    if(kioskView) {
        isLoggingEnabled = kioskView->bookmarkWidget()->isLoggingEnabled();
    }
    if(isLoggingEnabled) {
        WorldItem* worldItem = simulatorItem->findOwnerItem<WorldItem>();
        if(worldItem) {
            ItemList<WorldLogFileItem> logItems = worldItem->descendantItems<WorldLogFileItem>();
            WorldLogFileItemPtr logItem;
            if(logItems.size()) {
                logItem = logItems[0];
            } else {
                logItem = new WorldLogFileItem;
                worldItem->addChildItem(logItem);
            }
            if(recordingStartTime.isValid()) {
                logItem->setLogFile(filename);
                logItem->setTimeStampSuffixEnabled(false);
                logItem->setSelected(true);
            }

            ProjectManager* pm = ProjectManager::instance();
            pm->saveProject(projectFile);
            string memo = kioskView->bookmarkWidget()->memo();
            kioskView->logWidget()->addItem(projectFile, memo);
        }
    }
}


void KIOSKManagerImpl::onAxis(const int& id, const double& position)
{
    Command command = konamiCommand[password];
    if(id == command.id && (int)position == (int)command.position) {
        ++password;
    } else {
        password = 0;
    }
}


void KIOSKManagerImpl::onButton(const int& id, const bool& isPressed)
{
    if(id == Joystick::LOGO_BUTTON && isPressed) {
        if(enable_kiosk->isChecked()) {
            int ret = QMessageBox::question(MainWindow::instance(), _("KIOSK"), _("Would you like to return to the home screen?"));
            if(ret == QMessageBox::Yes) {
                if(simulatorItem) {
                    simulatorItem->stopSimulation(true);
                }
                loadProject(true);
            } else {
                hide_menuBar->setChecked(false);
            }
        }
    }

    Command command = konamiCommand[password];
    if(id == command.id && isPressed == command.isPressed) {
        if(password < 19) {
            ++password;
        } else if(password == 19) {
            hide_menuBar->setChecked(false);
            password = 0;
        }
    } else {
        password = 0;
    }
}


void KIOSKManagerImpl::store(Mapping& archive)
{
    archive.write("enable_kiosk", enable_kiosk->isChecked());
    archive.write("hide_menu_bar", hide_menuBar->isChecked());
    archive.write("hide_tool_bar", hide_toolBar->isChecked());
}


void KIOSKManagerImpl::restore(const Mapping& archive)
{
    enable_kiosk->setChecked(archive.get("enable_kiosk", false));
    hide_menuBar->setChecked(archive.get("hide_menu_bar", false));
    hide_toolBar->setChecked(archive.get("hide_tool_bar", false));
}
