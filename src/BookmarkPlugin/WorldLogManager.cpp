/**
   \file
   \author Kenta Suzuki
*/

#include "WorldLogManager.h"
#include <cnoid/Action>
#include <cnoid/Archive>
#include <cnoid/AppConfig>
#include <cnoid/Button>
#include <cnoid/Dialog>
#include <cnoid/MainWindow>
#include <cnoid/Menu>
#include <cnoid/MenuManager>
#include <cnoid/MessageView>
#include <cnoid/ProjectManager>
#include <cnoid/Separator>
#include <cnoid/SimulationBar>
#include <cnoid/SimulatorItem>
#include <cnoid/stdx/filesystem>
#include <cnoid/TimeBar>
#include <cnoid/TreeWidget>
#include <cnoid/UTF8>
#include <cnoid/WorldItem>
#include <src/BodyPlugin/WorldLogFileItem.h>
#include <QDateTime>
#include <QDialogButtonBox>
#include <QTreeWidgetItem>
#include <QVBoxLayout>
#include "gettext.h"

using namespace cnoid;
using namespace std;
namespace filesystem = cnoid::stdx::filesystem;

namespace {

WorldLogManager* instance_ = nullptr;
Action* enable_logging = nullptr;

}

namespace cnoid {

class WorldLogManagerImpl : public Dialog
{
public:
    WorldLogManagerImpl(ExtensionManager* ext, WorldLogManager* self);
    virtual ~WorldLogManagerImpl();
    WorldLogManager* self;

    TreeWidget* treeWidget;
    Menu contextMenu;

    void addItem(const string& filename);
    void removeItem();
    void onStartButtonClicked();
    void onCustomContextMenuRequested(const QPoint& pos);
    bool onOpenButtonClicked(const string& filename);
    void onSimulationAboutToStart(SimulatorItem* simulatorItem);
    void store(Mapping& archive);
    void restore(const Mapping& archive);
};

}


WorldLogManager::WorldLogManager(ExtensionManager* ext)
{
    impl = new WorldLogManagerImpl(ext, this);
}


WorldLogManagerImpl::WorldLogManagerImpl(ExtensionManager* ext, WorldLogManager* self)
    : self(self)
{
    setWindowTitle(_("WorldLogManager"));

    MenuManager& mm = ext->menuManager().setPath("/" N_("Options")).setPath(_("WorldLog"));
    enable_logging = mm.addCheckItem(_("Save a WorldLog"));

    setFixedSize(800, 450);
    treeWidget = new TreeWidget;
    treeWidget->setHeaderHidden(false);

    treeWidget->setHeaderLabel(_("File"));

    treeWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    Action* removeAct = new Action;
    removeAct->setText(_("Remove"));
    contextMenu.addAction(removeAct);
    Action* openAct = new Action;
    openAct->setText(_("Open"));
    contextMenu.addAction(openAct);

    removeAct->sigTriggered().connect([&](){ removeItem(); });
    openAct->sigTriggered().connect([&](){ onStartButtonClicked(); });
    connect(treeWidget, &TreeWidget::customContextMenuRequested,
        [=](const QPoint& pos){ onCustomContextMenuRequested(pos); });

    auto buttonBox = new QDialogButtonBox(this);
    auto startButton  = new PushButton(_("&Play"));
    startButton->setIconSize(MainWindow::instance()->iconSize());
    buttonBox->addButton(startButton, QDialogButtonBox::ActionRole);
    // connect(buttonBox, &QDialogButtonBox::accepted, [this](){ this->onStartButtonClicked(); });
    startButton->sigClicked().connect([&](){ onStartButtonClicked(); });

    QVBoxLayout* vbox = new QVBoxLayout;
    vbox->addWidget(treeWidget);
    vbox->addWidget(new HSeparator);
    vbox->addWidget(buttonBox);
    setLayout(vbox);

    SimulationBar* sb = SimulationBar::instance();
    sb->sigSimulationAboutToStart().connect(
                [&](SimulatorItem* simulatorItem){ onSimulationAboutToStart(simulatorItem); });

    Mapping& config = *AppConfig::archive()->openMapping("world_log_manager");
    if(config.isValid()) {
        restore(config);
    }
}


WorldLogManager::~WorldLogManager()
{
    delete impl;
}


WorldLogManagerImpl::~WorldLogManagerImpl()
{
    store(*AppConfig::archive()->openMapping("world_log_manager"));
}


void WorldLogManager::initializeClass(ExtensionManager* ext)
{
    if(!instance_) {
        instance_ = ext->manage(new WorldLogManager(ext));
    }

    MenuManager& mm = ext->menuManager().setPath("/" N_("Tools"));
    mm.addItem(_("WorldLogManager"))->sigTriggered().connect(
        [&](){ instance_->impl->show(); });
}


WorldLogManager* WorldLogManager::instance()
{
    return instance_;
}


void WorldLogManager::showWorldLogManagerDialog()
{
    instance_->impl->show();
}


void WorldLogManagerImpl::addItem(const string& filename)
{
    QTreeWidgetItem* item = new QTreeWidgetItem(treeWidget);
    item->setText(0, filename.c_str());
    treeWidget->setCurrentItem(item);
}


void WorldLogManagerImpl::removeItem()
{
    QTreeWidgetItem* item = treeWidget->currentItem();
    if(item) {
        int index = treeWidget->indexOfTopLevelItem(item);
        treeWidget->takeTopLevelItem(index);
    }
}


void WorldLogManagerImpl::onStartButtonClicked()
{
    QTreeWidgetItem* item = treeWidget->currentItem();
    if(item) {
        string filename = item->text(0).toStdString();
        onOpenButtonClicked(filename);
    }
}


void WorldLogManagerImpl::onCustomContextMenuRequested(const QPoint& pos)
{
    contextMenu.exec(treeWidget->mapToGlobal(pos));
}


bool WorldLogManagerImpl::onOpenButtonClicked(const string& filename)
{
    MessageView* mv = MessageView::instance();
    ProjectManager* pm = ProjectManager::instance();
    TimeBar* tb = TimeBar::instance();
    bool result = pm->tryToCloseProject();
    if(result) {
        pm->clearProject();
        mv->flush();
        pm->loadProject(filename);
        tb->stopPlayback(true);
        tb->startPlayback(0.0);
    }
    return result;
}


void WorldLogManagerImpl::onSimulationAboutToStart(SimulatorItem* simulatorItem)
{
    if(enable_logging->isChecked()) {
        filesystem::path homeDir(fromUTF8(getenv("HOME")));
        ProjectManager* pm = ProjectManager::instance();
        QDateTime recordingStartTime = QDateTime::currentDateTime();
        string suffix = recordingStartTime.toString("-yyyy-MM-dd-hh-mm-ss").toStdString();
        string logDirPath = toUTF8((homeDir / "worldlog" / (pm->currentProjectName() + suffix).c_str()).string());
        filesystem::path dir(fromUTF8(logDirPath));
        if(!filesystem::exists(dir)) {
            filesystem::create_directories(dir);
        }
        string filename0 = toUTF8((dir / pm->currentProjectName().c_str()).string()) + suffix + ".cnoid";

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
            addItem(filename0);
        }
    }
}


void WorldLogManagerImpl::store(Mapping& archive)
{
    archive.write("enable_logging", enable_logging->isChecked());

    int size = treeWidget->topLevelItemCount();
    archive.write("num_logs", size);
    for(int i = 0; i < size; ++i) {
        QTreeWidgetItem* item = treeWidget->topLevelItem(i);
        if(item) {
            string filename = item->text(0).toStdString();
            string fileKey = "file_name_" + to_string(i);
            archive.write(fileKey, filename);
        }
    }
}


void WorldLogManagerImpl::restore(const Mapping& archive)
{
    enable_logging->setChecked(archive.get("enable_logging", false));

    int size = archive.get("num_logs", 0);
    for(int i = 0; i < size; ++i) {
        string fileKey = "file_name_" + to_string(i);
        string filename = archive.get(fileKey, "");
        if(!filename.empty()) {
            addItem(filename);
        }
    }
}