/**
   @author Kenta Suzuki
*/

#include "TaskCreator.h"
#include <cnoid/AppConfig>
#include <cnoid/Archive>
#include <cnoid/Body>
#include <cnoid/BodyItem>
#include <cnoid/Buttons>
#include <cnoid/CheckBox>
#include <cnoid/ComboBox>
#include <cnoid/Dialog>
#include <cnoid/EigenTypes>
#include <cnoid/ExtensionManager>
#include <cnoid/ItemManager>
#include <cnoid/MainMenu>
#include <cnoid/ProjectManager>
#include <cnoid/RootItem>
#include <cnoid/Separator>
#include <cnoid/SimulationBar>
#include <cnoid/SpinBox>
#include <cnoid/SubProjectItem>
#include <cnoid/UTF8>
#include <cnoid/WorldItem>
#include <cnoid/stdx/filesystem>
#include <src/BodyPlugin/WorldLogFileItem.h>
#include <QBoxLayout>
#include <QDialogButtonBox>
#include <QLabel>
#include <QLineEdit>
#include <vector>
#include "gettext.h"

using namespace std;
using namespace cnoid;
namespace filesystem = cnoid::stdx::filesystem;

namespace {

TaskCreator* creatorInstance = nullptr;

}

namespace cnoid {

class TaskCreator::Impl : public Dialog
{
public:

    Impl();
    ~Impl();

    bool store(Archive& archive);
    void restore(const Archive& archive);
    void create();
    void start();
    void save();
    void onButton1Clicked(const int& id);
    void onButton2Clicked(const int& id);
    void onPosButtonClicked(const int& id);

    enum { NumProjects = 12 };

    ComboBox* projectCombos[NumProjects];
    CheckBox* logCheck;
    QLineEdit* logLine;
    DoubleSpinBox* posSpin;
};

}


void TaskCreator::initializeClass(ExtensionManager* ext)
{
    if(!creatorInstance) {
        creatorInstance = ext->manage(new TaskCreator);

        // ext->setProjectArchiver(
        //     "TaskCreator",
        //     [](Archive& archive){ return creatorInstance->impl->store(archive); },
        //     [](const Archive& archive) { return creatorInstance->impl->restore(archive); });

        MainMenu::instance()->add_Tools_Item(
            _("Task Creator"), [](){ creatorInstance->impl->show(); });
    }
}


TaskCreator* TaskCreator::instance()
{
    return creatorInstance;
}


TaskCreator::TaskCreator()
{
    impl = new Impl;
}


TaskCreator::Impl::Impl()
{
    setWindowTitle(_("Task Creator"));

    auto vbox = new QVBoxLayout;
    setLayout(vbox);

    auto hbox = new QHBoxLayout;

    for(int i = 0; i < NumProjects; ++i) {
        projectCombos[i] = new ComboBox;
        projectCombos[i]->setFixedWidth(480);

        auto button1 = new ToolButton("+");
        button1->sigClicked().connect([=](){ onButton1Clicked(i); });
        auto button2 = new ToolButton("-");
        button2->sigClicked().connect([=](){ onButton2Clicked(i); });
        auto button3 = new ToolButton("c");
        button3->sigClicked().connect([=](){ projectCombos[i]->clear(); });

        hbox->addWidget(new QLabel(QString(_("Project %1")).arg(i)));
        hbox->addWidget(projectCombos[i]);
        hbox->addWidget(button1);
        hbox->addWidget(button2);
        hbox->addWidget(button3);
        vbox->addLayout(hbox);
        hbox = new QHBoxLayout;
    }

    posSpin = new DoubleSpinBox;
    posSpin->setRange(0.0, 10.0);
    posSpin->setValue(1.5);
    hbox->addWidget(posSpin);

    const QStringList list = { "x+", "x-", "y+", "y-", "z+", "z-" };
    for(int i = 0; i < 6; ++i) {
        PushButton* button = new PushButton(list.at(i));
        button->sigClicked().connect([=](){ onPosButtonClicked(i); });
        hbox->addWidget(button);
    }

    vbox->addLayout(hbox);
    hbox = new QHBoxLayout;

    logCheck = new CheckBox;
    logCheck->setText(_("Log file"));
    // logCheck->setChecked(true);
    logLine = new QLineEdit;

    hbox->addWidget(logCheck);
    hbox->addWidget(logLine);
    vbox->addLayout(hbox);
    vbox->addStretch();

    vbox->addWidget(new HSeparator);

    PushButton* createButton = new PushButton(_("&Create"));
    createButton->setDefault(true);
    PushButton* startButton = new PushButton(_("&Start"));
    PushButton* saveButton = new PushButton(_("&Save"));
    QDialogButtonBox* buttonBox = new QDialogButtonBox(this);
    buttonBox->addButton(createButton, QDialogButtonBox::ActionRole);
    // buttonBox->addButton(startButton, QDialogButtonBox::ActionRole);
    // buttonBox->addButton(saveButton, QDialogButtonBox::ActionRole);
    createButton->sigClicked().connect([&](){ create(); });
    startButton->sigClicked().connect([&](){ start(); });
    saveButton->sigClicked().connect([&](){ save(); });

    vbox->addWidget(buttonBox);

    // restore config
    for(int i = 0; i < NumProjects; ++i) {
        string key = "registered_projects_" + to_string(i);
        projectCombos[i]->clear();

        auto& projectList = *AppConfig::archive()->findListing(key);
        if(projectList.isValid() && !projectList.empty()) {
            for(int j = 0; j < projectList.size(); ++j) {
                if(projectList[j].isString()) {
                    projectCombos[i]->addItem(projectList[j].toString().c_str());
                }
            }
        }
    }
}


TaskCreator::~TaskCreator()
{
    delete impl;
}


TaskCreator::Impl::~Impl()
{
    // store config
    for(int i = 0; i < NumProjects; ++i) {
        string key = "registered_projects_" + to_string(i);

        auto& projectList = *AppConfig::archive()->openListing(key);
        projectList.clear();

        for(int j = 0; j < projectCombos[i]->count(); ++j) {
            string filename = projectCombos[i]->itemText(j).toStdString();
            projectList.append(filename, DOUBLE_QUOTED);
        }

        if(projectList.empty()) {
            AppConfig::archive()->remove(key);
        }
    }
}


bool TaskCreator::Impl::store(Archive& archive)
{
    return true;
}


void TaskCreator::Impl::restore(const Archive& archive)
{

}


void TaskCreator::Impl::create()
{
    ProjectManager::instance()->clearProject();
    auto rootItem = RootItem::instance();

    WorldItem* worldItem = new WorldItem;
    rootItem->addChildItem(worldItem);
    for(int i = 0; i < NumProjects; ++i) {
        string filename = projectCombos[i]->currentText().toStdString();
        if(!filename.empty()) {
            SubProjectItem* projectItem = new SubProjectItem;
            filesystem::path path(fromUTF8(filename));
            projectItem->setName(path.stem());
            projectItem->load(filename);
            worldItem->addChildItem(projectItem);
        }
    }

    if(logCheck->isChecked()) {
        WorldLogFileItem* logItem = new WorldLogFileItem;
        string filename = logLine->text().toStdString();
        if(filename.empty()) {
            filename = "task";
        }
        filename += ".log";
        logItem->setLogFile(filename);
        logItem->setTimeStampSuffixEnabled(true);
        logItem->setRecordingFrameRate(100);
        worldItem->addChildItem(logItem);
    }
}


void TaskCreator::Impl::start()
{
    SimulationBar::instance()->startSimulation();
}


void TaskCreator::Impl::save()
{
    ProjectManager::instance()->showDialogToSaveProject();
}


void TaskCreator::Impl::onButton1Clicked(const int& id)
{
    vector<string> filenames = getOpenFileNames(_("Open a project"), "cnoid");
    projectCombos[id]->blockSignals(true);
    for(auto& filename : filenames) {
        projectCombos[id]->addItem(filename.c_str());
    }
    projectCombos[id]->blockSignals(false);
}


void TaskCreator::Impl::onButton2Clicked(const int& id)
{
    int currentIndex = projectCombos[id]->currentIndex();
    if(currentIndex != -1) {
        projectCombos[id]->removeItem(currentIndex);
    }
}


void TaskCreator::Impl::onPosButtonClicked(const int& id)
{
    auto rootItem = RootItem::instance();
    int index1 = id / 2;
    int index2 = id % 2;
    double pos = posSpin->value();
    pos = index2 == 0 ? pos : pos * -1.0;

    ItemList<BodyItem> bodyItems = rootItem->selectedItems<BodyItem>();
    for(auto& bodyItem : bodyItems) {
        auto body = bodyItem->body();

        Link* rootLink = body->rootLink();
        Vector3 p = rootLink->translation();
        p[index1] += pos;
        rootLink->setTranslation(p);
        bodyItem->notifyKinematicStateChange(true);
        bodyItem->storeInitialState();
    }
}
