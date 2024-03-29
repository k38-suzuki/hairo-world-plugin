/**
   @author Kenta Suzuki
*/

#include "WorldLogManager.h"
#include <cnoid/Archive>
#include <cnoid/AppConfig>
#include <cnoid/CheckBox>
#include <cnoid/ExtensionManager>
#include <cnoid/MainWindow>
#include <cnoid/ProjectManager>
#include <cnoid/SimulationBar>
#include <cnoid/SimulatorItem>
#include <cnoid/stdx/filesystem>
#include <cnoid/TimeBar>
#include <cnoid/UTF8>
#include <cnoid/WorldItem>
#include <src/BodyPlugin/WorldLogFileItem.h>
#include <QDateTime>
#include "gettext.h"

using namespace std;
using namespace cnoid;
namespace filesystem = cnoid::stdx::filesystem;

namespace {

WorldLogManager* logInstance = nullptr;

}


void WorldLogManager::initializeClass(ExtensionManager* ext)
{
    if(!logInstance) {
        logInstance = ext->manage(new WorldLogManager);

        vector<ToolBar*> toolBars = MainWindow::instance()->toolBars();
        for(auto& bar : toolBars) {
            if(bar->name() == "FileBar") {
                auto button1 = bar->addButton(QIcon::fromTheme("emblem-documents"));
                button1->setToolTip(_("Show the worldlog manager"));
                button1->sigClicked().connect([&](){
                    logInstance->updateList();
                    logInstance->show(); });
            }
        }
    }
}


WorldLogManager* WorldLogManager::instance()
{
    return logInstance;
}


WorldLogManager::WorldLogManager()
{
    setWindowTitle(_("WorldLogManager"));
    setArchiveKey("world_log_list");
    setFixedSize(800, 450);

    project_filename.clear();
    is_started = false;

    saveCheck = new CheckBox;
    saveCheck->setText(_("Save a WorldLog"));
    addWidget(saveCheck);

    TimeBar::instance()->sigPlaybackStopped().connect(
        [&](double time, bool isStoppedManually){ onPlaybackStopped(time, isStoppedManually); });

    SimulationBar::instance()->sigSimulationAboutToStart().connect(
        [&](SimulatorItem* simulatorItem){ onSimulationAboutToStart(simulatorItem); });

    auto archive = AppConfig::archive()->openMapping("world_log_manager");
    saveCheck->setChecked(archive->get("save_world_log", false));
}


WorldLogManager::~WorldLogManager()
{
    auto archive = AppConfig::archive()->openMapping("world_log_manager");
    archive->write("save_world_log", saveCheck->isChecked());
}


void WorldLogManager::onItemDoubleClicked(string& text)
{
    if(!text.empty()) {
        filesystem::path path(text);
        string extension = path.extension().string();
        if(extension == ".cnoid") {
            ProjectManager* pm = ProjectManager::instance();
            TimeBar* timeBar = TimeBar::instance();                
            bool result = pm->tryToCloseProject();
            if(result) {
                pm->clearProject();
                pm->loadProject(text);
                timeBar->stopPlayback(true);
                timeBar->startPlayback(0.0);
            }
        }
    }
}


void WorldLogManager::onSimulationAboutToStart(SimulatorItem* simulatorItem)
{
    is_started = true;
    if(saveCheck->isChecked()) {
        filesystem::path homeDir(fromUTF8(getenv("HOME")));
        ProjectManager* pm = ProjectManager::instance();
        QDateTime recordingStartTime = QDateTime::currentDateTime();
        string suffix = recordingStartTime.toString("-yyyy-MM-dd-hh-mm-ss").toStdString();
        string logDirPath = toUTF8((homeDir / "worldlog" / (pm->currentProjectName() + suffix).c_str()).string());
        filesystem::path dir(fromUTF8(logDirPath));
        if(!filesystem::exists(dir)) {
            filesystem::create_directories(dir);
        }

        project_filename = toUTF8((dir / pm->currentProjectName().c_str()).string()) + ".cnoid";
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
                string filename = toUTF8((dir / logItem->name().c_str()).string()) + ".log";
                logItem->setLogFile(filename);
                logItem->setTimeStampSuffixEnabled(false);
                logItem->setSelected(true);
            }
        }
    }
}


void WorldLogManager::onPlaybackStopped(double time, bool isStoppedManually)
{
    if(is_started) {
        ProjectManager::instance()->saveProject(project_filename);
        addItem(project_filename.c_str());
        is_started = false;
    }
}
