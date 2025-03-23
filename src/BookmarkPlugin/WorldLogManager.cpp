/**
   @author Kenta Suzuki
*/

#include "WorldLogManager.h"
#include <cnoid/AppConfig>
#include <cnoid/CheckBox>
#include <cnoid/ExtensionManager>
#include <cnoid/ProjectManager>
#include <cnoid/SimulationBar>
#include <cnoid/SimulatorItem>
#include <cnoid/TimeBar>
#include <cnoid/UTF8>
#include <cnoid/ValueTree>
#include <cnoid/WorldItem>
#include <cnoid/WorldLogFileItem>
#include <cnoid/stdx/filesystem>
#include <cnoid/LoggerUtil>
#include "HamburgerMenu.h"
#include "ProjectListedDialog.h"
#include "ListedWidget.h"
#include "gettext.h"

using namespace std;
using namespace cnoid;
namespace filesystem = cnoid::stdx::filesystem;


void WorldLogManager::initializeClass(ExtensionManager* ext)
{
    static WorldLogManager* widget = nullptr;

    if(!widget) {
        widget = ext->manage(new WorldLogManager);

        ProjectListedDialog::instance()->listedWidget()->addWidget(_("World Log"), widget);
    }
}


WorldLogManager::WorldLogManager(QWidget* parent)
    : ArchiveListWidget(parent),
      is_simulation_started_(false),
      project_filename_("")
{
    setWindowTitle(_("World Log Manager"));
    setArchiveKey("world_log_list");
    setMinimumSize(640, 480);

    saveCheck_ = new CheckBox;
    saveCheck_->setText(_("Save a World Log"));

    autoCheck_ = new CheckBox;
    autoCheck_->setText(_("Autoplay"));

    addWidget(saveCheck_);
    addWidget(autoCheck_);

    TimeBar::instance()->sigPlaybackStopped().connect(
        [&](double time, bool isStoppedManually){ onPlaybackStopped(time, isStoppedManually); });

    SimulationBar::instance()->sigSimulationAboutToStart().connect(
        [&](SimulatorItem* simulatorItem){ onSimulationAboutToStart(simulatorItem); });

    auto config = AppConfig::archive()->openMapping("world_log_manager");
    if(config->isValid()) {
        saveCheck_->setChecked(config->get("save_world_log", false));
        autoCheck_->setChecked(config->get("auto_play", false));
    }
}


WorldLogManager::~WorldLogManager()
{
    auto config = AppConfig::archive()->openMapping("world_log_manager");
    config->write("save_world_log", saveCheck_->isChecked());
    config->write("auto_play", autoCheck_->isChecked());
}


void WorldLogManager::onItemDoubleClicked(const string& text)
{
    if(loadProject(text)) {
        if(autoCheck_->isChecked()) {
            TimeBar* timeBar = TimeBar::instance();
            timeBar->stopPlayback(true);
            timeBar->startPlayback(0.0);
        }
    }
}


void WorldLogManager::onSimulationAboutToStart(SimulatorItem* simulatorItem)
{
    if(saveCheck_->isChecked()) {
        is_simulation_started_ = true;
        ProjectManager* pm = ProjectManager::instance();
        string suffix = getCurrentTimeSuffix();
        string log_dir = toUTF8(("worldlog" / filesystem::path(fromUTF8(pm->currentProjectName() + suffix))).string());
        filesystem::path logDirPath(fromUTF8(mkdirs(StandardPath::Downloads, log_dir)));

        project_filename_ = toUTF8((logDirPath / filesystem::path(fromUTF8(pm->currentProjectName() + ".cnoid"))).string());
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

            string filename = toUTF8((logDirPath / filesystem::path(fromUTF8(logItem->name() + ".log"))).string());
            logItem->setLogFile(filename);
            logItem->setTimeStampSuffixEnabled(false);
            logItem->setSelected(true);
        }
    }
}


void WorldLogManager::onPlaybackStopped(double time, bool isStoppedManually)
{
    if(saveCheck_->isChecked() && is_simulation_started_) {
        ProjectManager::instance()->saveProject(project_filename_);
        addItem(project_filename_.c_str());
        removeDuplicates();
    }
    is_simulation_started_ = false;
}