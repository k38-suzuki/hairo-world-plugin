/**
   \file
   \author Kenta Suzuki
*/

#include "WorldLogManagerDialog.h"
#include <cnoid/Action>
#include <cnoid/Archive>
#include <cnoid/AppConfig>
#include <cnoid/Button>
#include <cnoid/CheckBox>
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
#include <QHBoxLayout>
#include <QStyle>
#include <QTreeWidgetItem>
#include <QVBoxLayout>
#include "gettext.h"

using namespace cnoid;
using namespace std;
namespace filesystem = cnoid::stdx::filesystem;

namespace {

WorldLogManagerDialog* instance_ = nullptr;

}

namespace cnoid {

class WorldLogManagerDialogImpl
{
public:
    WorldLogManagerDialogImpl(WorldLogManagerDialog* self);
    virtual ~WorldLogManagerDialogImpl();
    WorldLogManagerDialog* self;

    TreeWidget* treeWidget;
    CheckBox* saveCheck;
    Menu contextMenu;
    string projectFileName;
    bool isSimulationStarted;

    void addItem(const string& filename);
    void removeItem();
    void onStartButtonClicked();
    void onCustomContextMenuRequested(const QPoint& pos);
    bool onOpenButtonClicked(const string& filename);
    void onSimulationAboutToStart(SimulatorItem* simulatorItem);
    void onPlaybackStopped(double time, bool isStoppedManually);
    void store(Mapping& archive);
    void restore(const Mapping& archive);
};

}


WorldLogManagerDialog::WorldLogManagerDialog()
{
    impl = new WorldLogManagerDialogImpl(this);
}


WorldLogManagerDialogImpl::WorldLogManagerDialogImpl(WorldLogManagerDialog* self)
    : self(self)
{
    self->setWindowTitle(_("WorldLogManager"));

    projectFileName.clear();
    isSimulationStarted = false;

    self->setFixedSize(800, 450);
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
    self->connect(treeWidget, &TreeWidget::customContextMenuRequested,
        [=](const QPoint& pos){ onCustomContextMenuRequested(pos); });

    QHBoxLayout* hbox = new QHBoxLayout;
    saveCheck = new CheckBox;
    saveCheck->setText(_("Save a WorldLog"));
    auto removeButton = new PushButton;
    removeButton->setIcon(QIcon(MainWindow::instance()->style()->standardIcon(QStyle::SP_TrashIcon)));
    removeButton->setToolTip(_("Remove project"));
    removeButton->sigClicked().connect([&](){ removeItem(); });
    hbox->addWidget(saveCheck);
    hbox->addStretch();
    hbox->addWidget(removeButton);

    auto buttonBox = new QDialogButtonBox(self);
    auto startButton  = new PushButton(_("&Play"));
    startButton->setIconSize(MainWindow::instance()->iconSize());
    buttonBox->addButton(startButton, QDialogButtonBox::ActionRole);
    // connect(buttonBox, &QDialogButtonBox::accepted, [this](){ this->onStartButtonClicked(); });
    startButton->sigClicked().connect([&](){ onStartButtonClicked(); });

    QVBoxLayout* vbox = new QVBoxLayout;
    vbox->addLayout(hbox);
    vbox->addWidget(treeWidget);
    vbox->addWidget(new HSeparator);
    vbox->addWidget(buttonBox);
    self->setLayout(vbox);

    TimeBar* timeBar = TimeBar::instance();
    timeBar->sigPlaybackStopped().connect([&](double time, bool isStoppedManually){ onPlaybackStopped(time, isStoppedManually); });

    SimulationBar* sb = SimulationBar::instance();
    sb->sigSimulationAboutToStart().connect(
                [&](SimulatorItem* simulatorItem){ onSimulationAboutToStart(simulatorItem); });

    Mapping& config = *AppConfig::archive()->openMapping("world_log_manager");
    if(config.isValid()) {
        restore(config);
    }
}


WorldLogManagerDialog::~WorldLogManagerDialog()
{
    delete impl;
}


WorldLogManagerDialogImpl::~WorldLogManagerDialogImpl()
{
    store(*AppConfig::archive()->openMapping("world_log_manager"));
}


WorldLogManagerDialog* WorldLogManagerDialog::instance()
{
    static WorldLogManagerDialog* instance_ = nullptr;
    if(!instance_) {
        instance_ = new WorldLogManagerDialog;
    }
    return instance_;
}


void WorldLogManagerDialog::showWorldLogManagerDialog()
{
    show();
}


void WorldLogManagerDialogImpl::addItem(const string& filename)
{
    QTreeWidgetItem* item = new QTreeWidgetItem(treeWidget);
    item->setText(0, filename.c_str());
    treeWidget->setCurrentItem(item);
}


void WorldLogManagerDialogImpl::removeItem()
{
    QTreeWidgetItem* item = treeWidget->currentItem();
    if(item) {
        int index = treeWidget->indexOfTopLevelItem(item);
        treeWidget->takeTopLevelItem(index);
    }
}


void WorldLogManagerDialogImpl::onStartButtonClicked()
{
    QTreeWidgetItem* item = treeWidget->currentItem();
    if(item) {
        string filename = item->text(0).toStdString();
        onOpenButtonClicked(filename);
    }
}


void WorldLogManagerDialogImpl::onCustomContextMenuRequested(const QPoint& pos)
{
    contextMenu.exec(treeWidget->mapToGlobal(pos));
}


bool WorldLogManagerDialogImpl::onOpenButtonClicked(const string& filename)
{
    MessageView* mv = MessageView::instance();
    ProjectManager* pm = ProjectManager::instance();
    TimeBar* timeBar = TimeBar::instance();
    bool result = pm->tryToCloseProject();
    if(result) {
        pm->clearProject();
        mv->flush();
        pm->loadProject(filename);
        timeBar->stopPlayback(true);
        timeBar->startPlayback(0.0);
    }
    return result;
}


void WorldLogManagerDialogImpl::onSimulationAboutToStart(SimulatorItem* simulatorItem)
{
    isSimulationStarted = true;
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

        projectFileName = toUTF8((dir / pm->currentProjectName().c_str()).string()) + ".cnoid";
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


void WorldLogManagerDialogImpl::onPlaybackStopped(double time, bool isStoppedManually)
{
    if(isSimulationStarted) {
        ProjectManager::instance()->saveProject(projectFileName);
        addItem(projectFileName);
        isSimulationStarted = false;
    }
}


void WorldLogManagerDialogImpl::store(Mapping& archive)
{
    archive.write("save_world_log_file", saveCheck->isChecked());

    int size = treeWidget->topLevelItemCount();
    archive.write("num_world_logs", size);
    for(int i = 0; i < size; ++i) {
        QTreeWidgetItem* item = treeWidget->topLevelItem(i);
        if(item) {
            string filename = item->text(0).toStdString();
            string fileKey = "filename_" + to_string(i);
            archive.write(fileKey, filename);
        }
    }
}


void WorldLogManagerDialogImpl::restore(const Mapping& archive)
{
    saveCheck->setChecked(archive.get("save_world_log_file", false));

    int size = archive.get("num_world_logs", 0);
    for(int i = 0; i < size; ++i) {
        string fileKey = "filename_" + to_string(i);
        string filename = archive.get(fileKey, "");
        if(!filename.empty()) {
            addItem(filename);
        }
    }
}