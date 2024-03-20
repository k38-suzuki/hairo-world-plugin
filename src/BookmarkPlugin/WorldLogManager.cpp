/**
   @author Kenta Suzuki
*/

#include "WorldLogManager.h"
#include <cnoid/Action>
#include <cnoid/Archive>
#include <cnoid/AppConfig>
#include <cnoid/Button>
#include <cnoid/CheckBox>
#include <cnoid/Dialog>
#include <cnoid/ExtensionManager>
#include <cnoid/MainWindow>
#include <cnoid/Menu>
#include <cnoid/MenuManager>
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
#include <QTreeWidgetItem>
#include <QVBoxLayout>
#include "gettext.h"

using namespace cnoid;
using namespace std;
namespace filesystem = cnoid::stdx::filesystem;

namespace {

WorldLogManager* logInstance = nullptr;

}

namespace cnoid {

class WorldLogManager::Impl : public Dialog
{
public:

    TreeWidget* treeWidget;
    CheckBox* saveCheck;
    Menu contextMenu;
    string projectFileName;
    bool isSimulationStarted;

    Impl();
    ~Impl();

    void addItem(const string& filename);
    void removeItem();
    void onStartButtonClicked();
    void onCustomContextMenuRequested(const QPoint& pos);
    void onSimulationAboutToStart(SimulatorItem* simulatorItem);
    void onPlaybackStopped(double time, bool isStoppedManually);
    void store(Mapping* archive);
    void restore(const Mapping* archive);
};

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
                button1->sigClicked().connect([&](){ logInstance->show(); });
            }
        }
    }
}


WorldLogManager* WorldLogManager::instance()
{
    return logInstance;
}


void WorldLogManager::show()
{
    impl->show();
}


WorldLogManager::WorldLogManager()
{
    impl = new Impl;
}


WorldLogManager::Impl::Impl()
{
    setWindowTitle(_("WorldLogManager"));

    projectFileName.clear();
    isSimulationStarted = false;

    setFixedSize(800, 450);
    treeWidget = new TreeWidget;
    treeWidget->setHeaderHidden(true);

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

    QHBoxLayout* hbox = new QHBoxLayout;
    saveCheck = new CheckBox;
    saveCheck->setText(_("Save a WorldLog"));
    auto removeButton = new PushButton;
    QIcon removeIcon = QIcon::fromTheme("user-trash");
    if(removeIcon.isNull()) {
        removeButton->setText(_("Remove"));
    } else {
        removeButton->setIcon(removeIcon);
    }
    removeButton->setToolTip(_("Remove project"));
    removeButton->sigClicked().connect([&](){ removeItem(); });
    hbox->addWidget(saveCheck);
    hbox->addStretch();
    hbox->addWidget(removeButton);

    auto buttonBox = new QDialogButtonBox(this);
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
    setLayout(vbox);

    TimeBar* timeBar = TimeBar::instance();
    timeBar->sigPlaybackStopped().connect([&](double time, bool isStoppedManually){ onPlaybackStopped(time, isStoppedManually); });

    SimulationBar* sb = SimulationBar::instance();
    sb->sigSimulationAboutToStart().connect(
                [&](SimulatorItem* simulatorItem){ onSimulationAboutToStart(simulatorItem); });

    auto config = AppConfig::archive()->openMapping("world_log_manager");
    if(config->isValid()) {
        restore(config);
    }
}


WorldLogManager::~WorldLogManager()
{
    delete impl;
}


WorldLogManager::Impl::~Impl()
{
    store(AppConfig::archive()->openMapping("world_log_manager"));
}


void WorldLogManager::Impl::addItem(const string& filename)
{
    QTreeWidgetItem* item = new QTreeWidgetItem(treeWidget);
    item->setText(0, filename.c_str());
    treeWidget->setCurrentItem(item);
}


void WorldLogManager::Impl::removeItem()
{
    QTreeWidgetItem* item = treeWidget->currentItem();
    if(item) {
        int index = treeWidget->indexOfTopLevelItem(item);
        treeWidget->takeTopLevelItem(index);
    }
}


void WorldLogManager::Impl::onStartButtonClicked()
{
    QTreeWidgetItem* item = treeWidget->currentItem();
    if(item) {
        string filename = item->text(0).toStdString();
        if(!filename.empty()) {
            filesystem::path path(filename);
            string extension = path.extension().string();
            if(extension == ".cnoid") {
                ProjectManager* pm = ProjectManager::instance();
                TimeBar* timeBar = TimeBar::instance();                
                bool result = pm->tryToCloseProject();
                if(result) {
                    pm->clearProject();
                    pm->loadProject(filename);
                    timeBar->stopPlayback(true);
                    timeBar->startPlayback(0.0);
                }
            }
        }
    }
}


void WorldLogManager::Impl::onCustomContextMenuRequested(const QPoint& pos)
{
    // contextMenu.exec(treeWidget->mapToGlobal(pos));
}


void WorldLogManager::Impl::onSimulationAboutToStart(SimulatorItem* simulatorItem)
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


void WorldLogManager::Impl::onPlaybackStopped(double time, bool isStoppedManually)
{
    if(isSimulationStarted) {
        ProjectManager::instance()->saveProject(projectFileName);
        addItem(projectFileName);
        isSimulationStarted = false;
    }
}


void WorldLogManager::Impl::store(Mapping* archive)
{
    archive->write("save_world_log_file", saveCheck->isChecked());

    ListingPtr logList = new Listing;
    for(int i = 0; i < treeWidget->topLevelItemCount(); ++i) {
        QTreeWidgetItem* item = treeWidget->topLevelItem(i);
        if(item) {
            string filename = item->text(0).toStdString();
            logList->append(filename, DOUBLE_QUOTED);
        }
    }
    if(!logList->size()) {
        logList->append("", DOUBLE_QUOTED);
    }
    AppConfig::archive()->insert("world_logs", logList);
}


void WorldLogManager::Impl::restore(const Mapping* archive)
{
    saveCheck->setChecked(archive->get("save_world_log_file", false));

    auto& logList = *AppConfig::archive()->findListing("world_logs");
    if(logList.isValid() && !logList.empty()) {
        for(int i = 0; i < logList.size(); ++i) {
            string filename = logList[i].toString();
            if(!filename.empty()) {
                addItem(filename);
            }
        }
    }
}
