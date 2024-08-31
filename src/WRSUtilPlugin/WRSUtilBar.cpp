/**
   @author Kenta Suzuki
*/

#include "WRSUtilBar.h"
#include <cnoid/AISTSimulatorItem>
#include <cnoid/BodyItem>
#include <cnoid/ComboBox>
#include <cnoid/EigenArchive>
#include <cnoid/ExecutablePath>
#include <cnoid/ExtensionManager>
#include <cnoid/Format>
#include <cnoid/ItemTreeView>
#include <cnoid/MessageView>
#include <cnoid/NullOut>
#include <cnoid/ProjectManager>
#include <cnoid/RootItem>
#include <cnoid/SimpleControllerItem>
#include <cnoid/SubProjectItem>
#include <cnoid/ToolButton>
#include <cnoid/WorldItem>
#include <cnoid/YAMLReader>
#include <src/BodyPlugin/WorldLogFileItem.h>
#include <vector>
#include "gettext.h"

using namespace std;
using namespace cnoid;

namespace {

struct ProjectInfo {
    string name;
    string view_project;
    string task_project;
    vector<string> simulator_projects;
    vector<string> robot_projects;
    bool is_recording_enabled;
    bool is_ros_enabled;
    Vector3 start_position;
};

}

namespace cnoid {

class WRSUtilBar::Impl
{
public:
    WRSUtilBar* self;

    Impl(WRSUtilBar* self);

    ComboBox* projectCombo;

    string project_dir;
    string registration_file;
    vector<ProjectInfo> projectInfo;
    bool is_initialized;

    bool load(const string& filename, ostream& os = nullout());
    void update();
    void onOpenButtonClicked();
    void onUpdateButtonClicked();
};

}


void WRSUtilBar::initialize(ExtensionManager* ext)
{
    static bool initialized = false;
    if(!initialized) {
        ext->addToolBar(instance());
        initialized = true;
    }
}


WRSUtilBar* WRSUtilBar::instance()
{
    static WRSUtilBar* utilBar = new WRSUtilBar;
    return utilBar;
}


WRSUtilBar::WRSUtilBar()
    : ToolBar(N_("WRSUtilBar"))
{
    impl = new Impl(this);
}


WRSUtilBar::WRSUtilBar(const string& name)
    : ToolBar(name)
{
    impl = new Impl(this);
}


WRSUtilBar::Impl::Impl(WRSUtilBar* self)
    : self(self),
      project_dir(""),
      registration_file(""),
      is_initialized(false)
{
    self->setVisibleByDefault(true);

    projectCombo = new ComboBox;
    projectCombo->setToolTip(_("Select project"));
    self->addWidget(projectCombo);

    auto updateButton = self->addButton(QIcon::fromTheme("view-refresh"));
    updateButton->setToolTip(_("Update projects"));
    updateButton->sigClicked().connect([&](){ onUpdateButtonClicked(); });

    auto openButton = self->addButton(QIcon::fromTheme("window-new"));
    openButton->setToolTip(_("Open the selected project"));
    openButton->sigClicked().connect([&](){ onOpenButtonClicked(); });
}


WRSUtilBar::~WRSUtilBar()
{
    delete impl;
}


void WRSUtilBar::setProjectDirectory(const std::string& directory)
{
    impl->project_dir = directory;
}


void WRSUtilBar::setRegistrationFile(const std::string& filename)
{
    impl->registration_file = filename;
}


void WRSUtilBar::update()
{
    impl->update();
}


void WRSUtilBar::Impl::update()
{
    if(!registration_file.empty()) {
        load(registration_file);
        is_initialized = true;
    }
}


bool WRSUtilBar::Impl::load(const string& filename, ostream& os)
{
    projectCombo->clear();
    projectInfo.clear();

    try {
        YAMLReader reader;
        MappingPtr node = reader.loadDocument(filename)->toMapping();
        if(node) {
            auto& registrationList = *node->findListing("registrations");
            if(registrationList.isValid()) {
                for(int i = 0; i < registrationList.size(); ++i) {
                    Mapping* node = registrationList[i].toMapping();
                    ProjectInfo info;

                    string name = node->get("name", "");
                    info.name = name;
                    projectCombo->addItem(name.c_str());

                    string task = node->get("task_project", "");
                    info.task_project = task;

                    string view = node->get("view_project", "");
                    info.view_project = view;

                    auto& simulatorList = *node->findListing("simulator_project");
                    if(simulatorList.isValid()) {
                        for(int j = 0; j < simulatorList.size(); ++j) {
                            string simulator = simulatorList[j].toString();
                            info.simulator_projects.push_back(simulator);
                        }
                    }

                    auto& robotList = *node->findListing("robot_project");
                    for(int j = 0; j < robotList.size(); ++j) {
                        string robot = robotList[j].toString();
                        info.robot_projects.push_back(robot);
                    }

                    if(!read(node, "start_position", info.start_position)) {

                    }

                    bool is_recording_enabled = node->get("enable_recording", true);
                    info.is_recording_enabled = is_recording_enabled;

                    bool is_ros_enabled = node->get("enable_ros", false);
                    info.is_ros_enabled = is_ros_enabled;

                    projectInfo.push_back(info);
                }
            }
        }
    }
    catch (const ValueNode::Exception& ex) {
        os << ex.message();
    }

    return true;
}


void WRSUtilBar::Impl::onOpenButtonClicked()
{
    if(project_dir.empty()) {
        return;
    } else if(!is_initialized) {
        return;
    }

    auto itemTreeView = ItemTreeView::instance();
    auto projectManager = ProjectManager::instance();
    auto rootItem = RootItem::instance();

    bool result = projectManager->tryToCloseProject();
    if(result) {
        projectManager->clearProject();
    } else {
        return;
    }

    int index = projectCombo->currentIndex();
    if(index == -1) {
        return;
    }

    ProjectInfo info = projectInfo[index];

    auto worldItem = new WorldItem;
    worldItem->setName("World");
    rootItem->addChildItem(worldItem);

    if(!info.task_project.empty()) {
        auto taskItem = new SubProjectItem();
        taskItem->setName(info.task_project);
        taskItem->load(project_dir + "/" + info.task_project + ".cnoid");
        worldItem->addChildItem(taskItem);
        itemTreeView->setExpanded(taskItem, false);
    }

    for(auto& project : info.simulator_projects) {
        projectManager->loadProject(project_dir + "/" + project + ".cnoid", worldItem);
        ItemList<SimulatorItem> simulatorItems = rootItem->selectedItems();
        for(auto& simulatorItem : simulatorItems) {
            simulatorItem->setSelected(false);
        }
    }

    Vector3 offset = info.start_position * -1.0;

    for(auto& project : info.robot_projects) {
        ItemList<BodyItem> loadedItems = projectManager->loadProject(project_dir + "/" + project + ".cnoid", worldItem);
        BodyItem* robotItem = nullptr;

        if(!loadedItems.size()) {
            MessageView::instance()->putln(formatR(_("Robots were not found.")));
        } else {
            robotItem = loadedItems[0];
        }

        if(robotItem) {
            auto rootLink = robotItem->body()->rootLink();
            auto p = rootLink->translation();
            p -= offset;
            rootLink->setTranslation(p);
            robotItem->notifyKinematicStateChange(true);
            robotItem->storeInitialState();
            offset[1] += 1.5;

            if(info.is_ros_enabled) {
                auto controllerItem = new SimpleControllerItem;
                controllerItem->setName(robotItem->name() + "-JoystickInput");
                auto mainControllerItem = robotItem->findItem<SimpleControllerItem>();
                mainControllerItem->addChildItem(controllerItem);

                controllerItem->setController("JoyTopicSubscriberController");
            }
        }
    }

    if(!info.view_project.empty()) {
        auto viewItem = new SubProjectItem();
        viewItem->setName(info.view_project);
        viewItem->load(project_dir + "/" + info.view_project + ".cnoid");
        rootItem->addChildItem(viewItem);
        itemTreeView->setExpanded(viewItem, false);
    }

    if(info.is_recording_enabled) {
        auto logItem = new WorldLogFileItem;
        logItem->setLogFile(info.task_project + ".log");
        logItem->setTimeStampSuffixEnabled(true);
        logItem->setRecordingFrameRate(100);
        worldItem->addChildItem(logItem);
    }

    projectManager->setCurrentProjectName(info.name);
}


void WRSUtilBar::Impl::onUpdateButtonClicked()
{
    update();
    MessageView::instance()->putln(formatR(_("Projects were updated.")));
}