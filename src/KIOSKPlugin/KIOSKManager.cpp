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
#include <cnoid/OptionManager>
#include <cnoid/ProjectManager>
#include <cnoid/SimulationBar>
#include <cnoid/SimulatorItem>
#include <cnoid/stdx/filesystem>
#include <cnoid/TimeBar>
#include <cnoid/UTF8>
#include <cnoid/ViewArea>
#include <cnoid/WorldItem>
#include <src/BodyPlugin/WorldLogFileItem.h>
#include <QDateTime>
#include <QMenuBar>
#include <QMessageBox>
#include <QStatusBar>
#include "src/Base/ToolBarArea.h"
#include "JoyKey.h"
#include "KIOSKView.h"
#include "gettext.h"

using namespace cnoid;
using namespace std;
namespace filesystem = cnoid::stdx::filesystem;

namespace {

Action* enable_kiosk = nullptr;
Action* enable_logging = nullptr;
Signal<void(bool)> sigLoggingEnabled_;

}


namespace cnoid {

class KIOSKManagerImpl
{
public:
    KIOSKManagerImpl(ExtensionManager* ext, KIOSKManager* self);
    KIOSKManager* self;
    virtual ~KIOSKManagerImpl();

    Action* hide_toolBar;
    bool isInitialized;
    JoystickCapture joystick;
    SimulatorItem* simulatorItem;
    JoyKey* key;

    void loadProject(const bool& enabled);
    void onProjectOptionsParsed(boost::program_options::variables_map& v);
    void onEnableKIOSKToggled(const bool& on);
    void onHideMenuBarToggled(const bool& on);
    void onHideToolBarToggled(const bool& on);
    void onEnableLoggingToggled(const bool& on);
    void onSimulationAboutToStart(SimulatorItem* simulatorItem);
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
    mm.setPath("/" N_("Options"));
    mm.setPath("KIOSK");
    enable_kiosk = mm.addCheckItem(_("Enable KIOSK"));
    enable_kiosk->sigToggled().connect([&](bool on){ onEnableKIOSKToggled(on); });
    enable_logging = mm.addCheckItem(_("Enable logging"));
    enable_logging->sigToggled().connect([&](bool on){ onEnableLoggingToggled(on); });

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


void KIOSKManager::initializeClass(ExtensionManager* ext)
{
    ext->manage(new KIOSKManager(ext));
    MainWindow::instance()->setFullScreen(false);
}


void KIOSKManager::setLoggingEnabled(const bool& on)
{
    enable_logging->blockSignals(true);
    enable_logging->setChecked(on);
    enable_logging->blockSignals(false);
}


SignalProxy<void(bool)> KIOSKManager::sigLoggingEnabled()
{
    return sigLoggingEnabled_;
}


void KIOSKManagerImpl::loadProject(const bool& enabled)
{
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
    onHideMenuBarToggled(on);
    hide_toolBar->setChecked(on);

    loadProject(on);
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


void KIOSKManagerImpl::onEnableLoggingToggled(const bool& on)
{
    sigLoggingEnabled_(on);
}


void KIOSKManagerImpl::onSimulationAboutToStart(SimulatorItem* simulatorItem)
{
    this->simulatorItem = simulatorItem;

    ProjectManager* pm = ProjectManager::instance();
    QDateTime recordingStartTime = QDateTime::currentDateTime();
    string suffix = recordingStartTime.toString("-yyyy-MM-dd-hh-mm-ss").toStdString();
    string logDirPath = toUTF8((shareDirPath() / "kiosk" / "log" / (pm->currentProjectName() + suffix).c_str()).string());
    filesystem::path dir(fromUTF8(logDirPath));
    if(!filesystem::exists(dir)) {
        filesystem::create_directories(dir);
    }
    string filename0 = toUTF8((dir / pm->currentProjectName().c_str()).string()) + suffix + ".cnoid";

    if(enable_logging->isChecked()) {
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
                string filename1 = toUTF8((dir / logItem->name().c_str()).string()) + suffix + ".log";
                logItem->setLogFile(filename1);
                logItem->setTimeStampSuffixEnabled(false);
                logItem->setSelected(true);
            }
            pm->saveProject(filename0);
            KIOSKView* kioskView = KIOSKView::instance();
            if(kioskView) {
                string memo = kioskView->bookmarkWidget()->memo();
                kioskView->logWidget()->addItem(filename0, memo);
            }
        }
    }
}


void KIOSKManagerImpl::onButton(const int& id, const bool& isPressed)
{
    if(id == Joystick::LOGO_BUTTON && isPressed) {
        if(enable_kiosk->isChecked()) {
            int ret = QMessageBox::question(MainWindow::instance(),
                                            _("KIOSK"), _("Would you like to return to the home screen?"));
            if(ret == QMessageBox::Yes) {
                loadProject(true);
            } else {
                onHideMenuBarToggled(false);
            }
        }
    }
}


void KIOSKManagerImpl::store(Mapping& archive)
{
    archive.write("enable_logging", enable_logging->isChecked());
}


void KIOSKManagerImpl::restore(const Mapping& archive)
{
    enable_logging->setChecked(archive.get("enable_logging", false));
}
