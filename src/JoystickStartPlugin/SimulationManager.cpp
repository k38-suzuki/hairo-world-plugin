/**
   \file
   \author Kenta Suzuki
*/

#include "SimulationManager.h"
#include <cnoid/Action>
#include <cnoid/AppConfig>
#include <cnoid/ExecutablePath>
#include <cnoid/FileDialog>
#include <cnoid/FileUtil>
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
#include <cnoid/stdx/filesystem>
#include <QMessageBox>
#include <fmt/format.h>
#include <functional>
#include "gettext.h"

using namespace std;
using namespace cnoid;
namespace filesystem = stdx::filesystem;
using fmt::format;

namespace {

SimulationManager* simulationManager = nullptr;
Action* useJoystickStart;
Action* useJoystickLoad;

}


namespace cnoid {

class SimulationManagerImpl
{
public:
  SimulationManagerImpl(SimulationManager* self);
  virtual ~SimulationManagerImpl();
  SimulationManager* self;

  bool start = false;
  bool pause = false;
  string currentProjectName;
  string currentProjectFile;
  JoystickCapture* joystick;

  void startSimulation(bool doReset = true);
  void startSimulation(SimulatorItem* simulatorItem, bool doReset);
  void stopSimulation(SimulatorItem* simulatorItem);
  void pauseSimulation(SimulatorItem* simulatorItem);
  void forEachSimulator(std::function<void(SimulatorItem* simulatorItem)> callback, bool doSelect = false);
  void onStopSimulationClicked();
  void onPauseSimulationClicked();
  void openDialogToLoadProject();
  void onButtonPressed(const int& id, const bool& isPressed);
};

}


SimulationManager::SimulationManager()
{
    impl = new SimulationManagerImpl(this);
}


SimulationManagerImpl::SimulationManagerImpl(SimulationManager* self)
    : self(self)
{
    joystick = new JoystickCapture();
    joystick->setDevice("/dev/input/js0");
    joystick->sigButton().connect([&](int id, bool isPressed){ onButtonPressed(id, isPressed); });
}


SimulationManager::~SimulationManager()
{
    delete impl;
}


SimulationManagerImpl::~SimulationManagerImpl()
{
    Mapping* config = AppConfig::archive()->openMapping("joystick_start");
    config->write("use_joystick_start", useJoystickStart->isChecked());
    config->write("use_joystick_load", useJoystickLoad->isChecked());
}


void SimulationManager::initializeClass(ExtensionManager* ext)
{
    if(!simulationManager) {
        simulationManager = ext->manage(new SimulationManager());
    }

    MenuManager& manager = ext->menuManager().setPath("/Options").setPath(N_("JoystickStart"));
    Mapping* config = AppConfig::archive()->openMapping("joystick_start");
    useJoystickStart = manager.addCheckItem(_("Use JoystickStart"));
    useJoystickLoad = manager.addCheckItem(_("Use JoystickLoad"));
    useJoystickStart->setChecked(config->get("use_joystick_start", false));
    useJoystickLoad->setChecked(config->get("use_joystick_load", false));
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
        TimeBar::instance()->startPlayback();
    } else {
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
    if(pause) {
        if(simulatorItem->isRunning()) {
            simulatorItem->pauseSimulation();
        }
        if(timeBar->isDoingPlayback()) {
            timeBar->stopPlayback();
        }
    } else {
        if(simulatorItem->isRunning()) {
            simulatorItem->restartSimulation();
        }
        timeBar->startPlayback();
    }
}


void SimulationManagerImpl::forEachSimulator(std::function<void(SimulatorItem* simulatorItem)> callback, bool doSelect)
{
    MessageView* mv = MessageView::instance();
    RootItem* rootItem = RootItem::instance();
    ItemList<SimulatorItem> simulatorItems = rootItem->selectedItems<SimulatorItem>();

    if(simulatorItems.empty()) {
        simulatorItems.extractChildItems(RootItem::instance());
        if(simulatorItems.empty()) {
            mv->notify(_("There is no simulator item."));
        } else if(simulatorItems.size() > 1) {
            simulatorItems.clear();
            mv->notify(_("Please select a simulator item to simulate."));
        } else {
            if(doSelect) {
                rootItem->selectItem(simulatorItems.front());
            }
        }
    }

    typedef map<WorldItem*, SimulatorItem*> WorldToSimulatorMap;
    WorldToSimulatorMap worldToSimulator;

    for(size_t i = 0; i < simulatorItems.size(); i++) {
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
        } else {
            WorldToSimulatorMap::iterator p = worldToSimulator.find(world);
            if(p != worldToSimulator.end()) {
                if(!p->second) {
                    mv->notify(format(_("{} cannot be processed because another simulator"
                                        "in the same world is also selected."),
                                      simulatorItem->name()));
                } else {
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
    ProjectManager* pm = ProjectManager::instance();
    MessageView* mv = MessageView::instance();
    auto mw = MainWindow::instance();
    int numItems = RootItem::instance()->countDescendantItems();
    if(numItems > 0) {
        QString title = _("Warning");
        QString message;
        QMessageBox::StandardButton clicked;
        if(currentProjectFile.empty()) {
            if(numItems == 1) {
                message = _("A project item exists. "
                            "Do you want to save it as a project file before loading a new project?");
            } else {
                message = _("Project items exist. "
                            "Do you want to save them as a project file before loading a new project?");
            }
            clicked = QMessageBox::warning(
                mw, title, message, QMessageBox::Save | QMessageBox::Cancel | QMessageBox::Ignore);
        } else {
            message = _("Project \"%1\" exists. Do you want to save it before loading a new project?");
            clicked = QMessageBox::warning(
                mw, title, message.arg(currentProjectName.c_str()),
                QMessageBox::Save | QMessageBox::Cancel | QMessageBox::Ignore);
        }
        if(clicked == QMessageBox::Cancel) {
            return;
        }
        if(clicked == QMessageBox::Save) {
            pm->overwriteCurrentProject();
        }
    }

    FileDialog dialog(mw);
    dialog.setWindowTitle(_("Open a Choreonoid project file"));
    dialog.setFileMode(QFileDialog::ExistingFile);
    dialog.setViewMode(QFileDialog::List);
    dialog.setLabelText(QFileDialog::Accept, _("Open"));
    dialog.setLabelText(QFileDialog::Reject, _("Cancel"));

    QStringList filters;
    filters << _("Project files (*.cnoid)");
    filters << _("Any files (*)");
    dialog.setNameFilters(filters);

    dialog.updatePresetDirectories();

    if(dialog.exec()) {
        pm->clearProject();
        mv->flush();
        string filename = dialog.selectedFiles().front().toStdString();
        pm->loadProject(filename, nullptr);
    }
}


void SimulationManagerImpl::onButtonPressed(const int& id, const bool& isPressed)
{
    if(isPressed) {
        switch (id) {
        case Joystick::START_BUTTON:
            if(useJoystickStart->isChecked()) {
                if(!start) {
                    //start
                    startSimulation(true);
                    start = !start;
                } else {
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
                } else {
                    //stop
                    onStopSimulationClicked();
                    start = false;
                    pause = false;
                }
            }
            break;
        case Joystick::LOGO_BUTTON:
            if(useJoystickLoad->isChecked()) {
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
