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
#include <cnoid/ValueTree>
#include <cnoid/WorldItem>
#include <cnoid/LoggerUtil>
#include <src/BodyPlugin/WorldLogFileItem.h>
#include "HamburgerMenu.h"
#include "gettext.h"

using namespace std;
using namespace cnoid;

namespace {

WorldLogManager* logInstance = nullptr;

}


void WorldLogManager::initializeClass(ExtensionManager* ext)
{
    if(!logInstance) {
        logInstance = ext->manage(new WorldLogManager);

        const QIcon icon = QIcon(":/GoogleMaterialSymbols/icon/restore_page_24dp_5F6368_FILL1_wght400_GRAD0_opsz24.svg");
        auto action = new Action;
        action->setText(_("World Log Manager"));
        action->setIcon(icon);
        action->setToolTip(_("Show the world log manager"));
        action->sigTriggered().connect([&](){ logInstance->show(); });
        HamburgerMenu::instance()->addAction(action);
    }
}


WorldLogManager::WorldLogManager()
    : ArchiveListDialog(),
      is_simulation_started_(false),
      project_filename_("")
{
    setWindowTitle(_("World Log Manager"));
    setArchiveKey("world_log_list");
    setFixedSize(800, 450);

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
        string dir = mkdirs(StandardPath::Downloads, "worldlog/" + pm->currentProjectName() + suffix);

        project_filename_ = dir + "/" + pm->currentProjectName() + ".cnoid";
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

            string filename = dir + "/" + logItem->name() + ".log";
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