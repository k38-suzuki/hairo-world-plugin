/**
   \file
   \author Kenta Suzuki
*/

#include "SimulationManager.h"
#include <cnoid/Action>
#include <cnoid/AppConfig>
#include <cnoid/ExecutablePath>
#include <cnoid/FileUtil>
#include <cnoid/ItemTreeView>
#include <cnoid/Joystick>
#include <cnoid/JoystickCapture>
#include <cnoid/MainWindow>
#include <cnoid/MenuManager>
#include <cnoid/MessageView>
#include <cnoid/ProjectManager>
#include <cnoid/RootItem>
#include <cnoid/SimulatorItem>
#include <cnoid/TimeBar>
#include <cnoid/WorldItem>
#include <QFileDialog>
#include <boost/filesystem.hpp>
#include <fmt/format.h>
#include <functional>
#include "gettext.h"

using namespace std;
using namespace cnoid;
namespace filesystem = boost::filesystem;
using fmt::format;

namespace {

Action* useJoystickStart;
Action* useJoystickLoad;
JoystickCapture joystick;

}


namespace cnoid {

class SimulationManagerImpl
{
public:
  SimulationManagerImpl(SimulationManager* self);

  SimulationManager* self;
  bool start = false;
  bool pause = false;
  void startSimulation(bool doReset = true);
  void startSimulation(SimulatorItem* simulatorItem, bool doReset);
  void stopSimulation(SimulatorItem* simulatorItem);
  void pauseSimulation(SimulatorItem* simulatorItem);
  void forEachSimulator(std::function<void(SimulatorItem* simulatorItem)> callback, bool doSelect = false);
  void onStopSimulationClicked();
  void onPauseSimulationClicked();
  void openDialogToLoadProject();
  void onButtonClicked(const int id, const bool isPressed);
};

}


SimulationManager::SimulationManager()
{
    impl = new SimulationManagerImpl(this);
}


SimulationManagerImpl::SimulationManagerImpl(SimulationManager* self)
    : self(self)
{

}


SimulationManager::~SimulationManager()
{
    delete impl;
}


void SimulationManager::initializeClass(ExtensionManager* ext)
{
    SimulationManager* gameManager = new SimulationManager();
    joystick.setDevice("/dev/input/js0");
    joystick.sigButton().connect([&, gameManager](int id, bool isPressed){ gameManager->onButtonClicked(id, isPressed); });

    MenuManager& manager = ext->menuManager().setPath("/Options").setPath(N_("JoystickStart"));
    Mapping* config = AppConfig::archive()->openMapping("JoystickStart");
    useJoystickStart = manager.addCheckItem(_("Use JoystickStart"));
    useJoystickStart->setChecked(config->get("useJoystickStart", false));
    useJoystickLoad = manager.addCheckItem(_("Use JoystickLoad"));
    useJoystickLoad->setChecked(config->get("useJoystickLoad", false));
}


void SimulationManager::finalizeClass()
{
    Mapping* config = AppConfig::archive()->openMapping("JoystickStart");
    config->write("useJoystickStart", useJoystickStart->isChecked());
    config->write("useJoystickLoad", useJoystickLoad->isChecked());
}


void SimulationManagerImpl::startSimulation(bool doReset)
{
    forEachSimulator([&, doReset](SimulatorItem* simulatorItem){ startSimulation(simulatorItem, doReset); }, true);
}


void SimulationManagerImpl::startSimulation(SimulatorItem* simulatorItem, bool doReset)
{
    if(simulatorItem->isRunning()) {
        if(pause && !doReset) {
            simulatorItem->restartSimulation();
            pause = false;
        }
        TimeBar::instance()->startPlaybackFromFillLevel();
    }
    else {
        simulatorItem->startSimulation(doReset);
        pause = false;
    }
}


void SimulationManagerImpl::stopSimulation(SimulatorItem* simulatorItem)
{
    simulatorItem->stopSimulation();
}


void SimulationManagerImpl::pauseSimulation(SimulatorItem* simulatorItem)
{
    TimeBar* timeBar = TimeBar::instance();
    if(pause){
        if(simulatorItem->isRunning()) {
            simulatorItem->pauseSimulation();
        }
        if(timeBar->isDoingPlayback()) {
            timeBar->stopPlayback();
        }
    }
    else {
        if(simulatorItem->isRunning()) {
            simulatorItem->restartSimulation();
        }
        timeBar->startPlaybackFromFillLevel();
    }
}


void SimulationManagerImpl::forEachSimulator(std::function<void(SimulatorItem* simulatorItem)> callback, bool doSelect)
{
    MessageView* mv = MessageView::instance();
    ItemList<SimulatorItem> simulatorItems = ItemTreeView::mainInstance()->selectedItems<SimulatorItem>();

    if(simulatorItems.empty()) {
        simulatorItems.extractChildItems(RootItem::instance());
        if(simulatorItems.empty()) {
            mv->notify(_("There is no simulator item."));
        }
        else if(simulatorItems.size() > 1) {
            simulatorItems.clear();
            mv->notify(_("Please select a simulator item to simulate."));
        }
        else {
            if(doSelect) {
                ItemTreeView::instance()->selectItem(simulatorItems.front());
            }
        }
    }

    typedef map<WorldItem*, SimulatorItem*> WorldToSimulatorMap;
    WorldToSimulatorMap worldToSimulator;

    for(size_t i = 0; i < simulatorItems.size(); i++){
        SimulatorItem* simulatorItem = simulatorItems.get(i);
        WorldItem* world = simulatorItem->findOwnerItem<WorldItem>();
        if(world) {
            WorldToSimulatorMap::iterator p = worldToSimulator.find(world);
            if(p == worldToSimulator.end()) {
                worldToSimulator[world] = simulatorItem;
            }
            else {
                p->second = 0;
            }
        }
    }

    for(size_t i = 0; i < simulatorItems.size(); i++) {
        SimulatorItem* simulatorItem = simulatorItems.get(i);
        WorldItem* world = simulatorItem->findOwnerItem<WorldItem>();
        if(!world) {
            mv->notify(format(_("{} cannot be processed because it is not related with a world."), simulatorItem->name()));
        }
        else {
            WorldToSimulatorMap::iterator p = worldToSimulator.find(world);
            if(p != worldToSimulator.end()) {
                if(!p->second) {
                    mv->notify(format(_("{} cannot be processed because another simulator"
                                        "in the same world is also selected."),
                                      simulatorItem->name()));
                }
                else {
                    callback(simulatorItem);
                }
            }
        }
    }
}


void SimulationManagerImpl::onStopSimulationClicked()
{
    forEachSimulator([&](SimulatorItem* simulatorItem){ stopSimulation(simulatorItem); });
    TimeBar* timeBar = TimeBar::instance();
    if(timeBar->isDoingPlayback()) {
        timeBar->stopPlayback();
    }
    pause = false;
}


void SimulationManagerImpl::onPauseSimulationClicked()
{
    forEachSimulator([&](SimulatorItem* simulatorItem){ pauseSimulation(simulatorItem); });
}


void SimulationManagerImpl::openDialogToLoadProject()
{
    QFileDialog dialog(MainWindow::instance());
    dialog.setWindowTitle(_("Open a Choreonoid project file"));
    dialog.setFileMode(QFileDialog::ExistingFile);
    dialog.setViewMode(QFileDialog::List);
    dialog.setLabelText(QFileDialog::Accept, _("Open"));
    dialog.setLabelText(QFileDialog::Reject, _("Cancel"));

    QStringList filters;
    filters << _("Project files (*.cnoid)");
    filters << _("Any files (*)");
    dialog.setNameFilters(filters);

    dialog.setDirectory(AppConfig::archive()->get
                        ("currentFileDialogDirectory", shareDirectory()).c_str());

    if(dialog.exec()){
        AppConfig::archive()->writePath("currentFileDialogDirectory", dialog.directory().absolutePath().toStdString());
        string filename = getNativePathString(filesystem::path(dialog.selectedFiles().front().toStdString()));
        ProjectManager::instance()->loadProject(filename, nullptr);
    }
}


void SimulationManager::onButtonClicked(const int id, const bool isPressed)
{
    impl->onButtonClicked(id, isPressed);
}


void SimulationManagerImpl::onButtonClicked(const int id, const bool isPressed)
{
    if(isPressed) {
        switch (id) {
        case Joystick::START_BUTTON:
            if(useJoystickStart->isChecked()) {
                if(!start) {
                    //start
                    startSimulation(true);
                    start = !start;
                }
                else {
                    //pause
                    pause = !pause;
                    onPauseSimulationClicked();
                }
            }
            break;
        case Joystick::SELECT_BUTTON:
            if(useJoystickStart->isChecked()) {
                if(!start && !pause) {
                    //re-start
                    startSimulation(false);
                    start = !start;
                }
                else {
                    //stop
                    onStopSimulationClicked();
                    start = false;
                    pause = false;
                }
            }
            break;
        case Joystick::LOGO_BUTTON:
            if(useJoystickLoad->isChecked()) {
                ItemTreeView* view = ItemTreeView::instance();
                view->selectAllItems();
                view->itemTreeWidget()->cutSelectedItems();
                view->clearSelection();
                openDialogToLoadProject();
                start = false;
                pause = false;
            }
            break;
        default:
            break;
        }
    }
}
