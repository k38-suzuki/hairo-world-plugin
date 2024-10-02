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
#include <cnoid/OptionManager>
#include <cnoid/ProjectManager>
#include <cnoid/RootItem>
#include <cnoid/SimpleControllerItem>
#include <cnoid/SubProjectItem>
#include <cnoid/ToolButton>
#include <cnoid/UTF8>
#include <cnoid/WorldItem>
#include <cnoid/YAMLReader>
#include <src/BodyPlugin/WorldLogFileItem.h>
#include <vector>
#include "gettext.h"

using namespace std;
using namespace cnoid;

namespace {

vector<string> projectToExecute;

struct ProjectInfo {
    string name;
    string view_project;
    string robot_alignment;
    vector<string> task_projects;
    vector<string> simulator_projects;
    vector<string> robot_projects;
    bool is_recording_enabled;
    bool is_ros_enabled;
    Vector3 start_position;
};

struct AlignmentInfo {
    Vector3 R;
    int offset_id;
    double offset_value;
};

AlignmentInfo alignmentInfo[] = {
    {   Vector3(0.0, 0.0, 0.0), 1,  1.5 },
    { Vector3(0.0, 0.0, 180.0), 1, -1.5 },
    {  Vector3(0.0, 0.0, 90.0), 0, -1.5 },
    { Vector3(0.0, 0.0, -90.0), 0,  1.5 },
    {   Vector3(0.0, 0.0, 0.0), 2, -1.5 },
    { Vector3(0.0, 0.0, 180.0), 2, -1.5 },
    {  Vector3(0.0, 0.0, 90.0), 2, -1.5 },
    { Vector3(0.0, 0.0, -90.0), 2, -1.5 },
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

    void onUtilOptionsParsed();
    void onInputFileOptionsParsed(vector<string>& inputFiles);

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
    self->setVisibleByDefault(false);

    auto om = OptionManager::instance();
    om->add_option("--wrs-util", projectToExecute, "execute registered project");
    om->sigInputFileOptionsParsed().connect(
        [this](vector<string>& inputFiles){ onInputFileOptionsParsed(inputFiles); });
    om->sigOptionsParsed(1).connect(
        [&](OptionManager*){ onUtilOptionsParsed(); });

    projectCombo = new ComboBox;
    projectCombo->setToolTip(_("Select a project"));
    self->addWidget(projectCombo);

    auto updateButton = self->addButton(":/GoogleMaterialSymbols/icon/refresh_24dp_5F6368_FILL1_wght400_GRAD0_opsz24.svg");
    updateButton->setToolTip(_("Update projects"));
    updateButton->sigClicked().connect([&](){ onUpdateButtonClicked(); });

    auto openButton = self->addButton(":/GoogleMaterialSymbols/icon/open_in_new_24dp_5F6368_FILL1_wght400_GRAD0_opsz24.svg");
    openButton->setToolTip(_("Open the selected project"));
    openButton->sigClicked().connect([&](){ onOpenButtonClicked(); });
}


WRSUtilBar::~WRSUtilBar()
{
    delete impl;
}


void WRSUtilBar::setProjectDirectory(const string& directory)
{
    impl->project_dir = directory;
}


void WRSUtilBar::setRegistrationFile(const string& filename)
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


void WRSUtilBar::Impl::onUtilOptionsParsed()
{
    for(auto& project : projectToExecute) {
        for(int i = 0; i < projectCombo->count(); ++i) {
            string text = projectCombo->itemText(i).toStdString();
            if(text == project) {
                projectCombo->setCurrentIndex(i);
                onOpenButtonClicked();
            }
        }
    }
}


void WRSUtilBar::Impl::onInputFileOptionsParsed(vector<string>& inputFiles)
{
    auto it = inputFiles.begin();
    while(it != inputFiles.end()) {
        if(filesystem::path(fromUTF8(*it)).extension().string() == ".yaml") {
            self->setVisibleByDefault(true);
            self->setRegistrationFile(*it);
            self->update();
            it = inputFiles.erase(it);
        } else {
            ++it;
        }
    }
}


bool WRSUtilBar::Impl::load(const string& filename, ostream& os)
{
    projectCombo->clear();
    projectInfo.clear();

    try {
        YAMLReader reader;
        auto archive = reader.loadDocument(filename)->toMapping();
        if(archive) {
            auto& registrationList = *archive->findListing("registrations");
            if(registrationList.isValid()) {
                for(int i = 0; i < registrationList.size(); ++i) {
                    Mapping* node = registrationList[i].toMapping();
                    ProjectInfo info;

                    string name = node->get("name", "");
                    info.name = name;
                    projectCombo->addItem(name.c_str());

                    string view = node->get("view_project", "");
                    info.view_project = view;

                    string robot_alignment = node->get("robot_alignment", "X+");
                    info.robot_alignment = robot_alignment;

                    auto& taskList = *node->findListing("task_project");
                    if(taskList.isValid()) {
                        for(int j = 0; j < taskList.size(); ++j) {
                            string task = taskList[j].toString();
                            info.task_projects.push_back(task);
                        }
                    }

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

                    if(read(node, "start_position", info.start_position)) {

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

    for(auto& project : info.task_projects) {
        auto taskItem = new SubProjectItem();
        taskItem->setName(project);
        taskItem->load(project_dir + "/" + project + ".cnoid");
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

    string alignment = info.robot_alignment;
    AlignmentInfo selectedInfo;
    if(alignment == "X+") {
        selectedInfo = alignmentInfo[0];
    } else if(alignment == "X-") {
        selectedInfo = alignmentInfo[1];
    } else if(alignment == "Y+") {
        selectedInfo = alignmentInfo[2];
    } else if(alignment == "Y-") {
        selectedInfo = alignmentInfo[3];
    } else if(alignment == "X+Z+") {
        selectedInfo = alignmentInfo[4];
    } else if(alignment == "X-Z+") {
        selectedInfo = alignmentInfo[5];
    } else if(alignment == "Y+Z+") {
        selectedInfo = alignmentInfo[6];
    } else if(alignment == "Y-Z+") {
        selectedInfo = alignmentInfo[7];
    }

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
            rootLink->setRotation(rotFromRpy(radian(selectedInfo.R)));
            robotItem->notifyKinematicStateChange(true);
            robotItem->storeInitialState();
            // offset[1] += 1.5;
            offset[selectedInfo.offset_id] += selectedInfo.offset_value;

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
        logItem->setLogFile(info.name + ".log");
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