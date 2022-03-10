/**
   \file
   \author Kenta Suzuki
*/

#include "HistoryManager.h"
#include <cnoid/Action>
#include <cnoid/AppConfig>
#include <cnoid/Menu>
#include <cnoid/MenuManager>
#include <cnoid/MessageView>
#include <cnoid/ProjectManager>
#include "gettext.h"

using namespace cnoid;
using namespace std;

namespace cnoid {

class HistoryManagerImpl
{
public:
    HistoryManagerImpl(HistoryManager* self, ExtensionManager* ext);
    virtual ~HistoryManagerImpl();
    HistoryManager* self;

    Menu* currentMenu;
    Menu* contextMenu;
    Action* clearProject;
    ProjectManager* pm;
    QPoint pos;

    void onClearProjectTriggered();
    void onRemoveProjectTriggered();
    void onCurrentMenuTriggered(QAction* action);
    void onCustomContextMenuRequested(const QPoint& pos);
    void onProjectLoaded();
    void store(Mapping& archive);
    void restore(const Mapping& archive);
};

}


HistoryManager::HistoryManager(ExtensionManager* ext)
{
    impl = new HistoryManagerImpl(this, ext);
}


HistoryManagerImpl::HistoryManagerImpl(HistoryManager* self, ExtensionManager* ext)
    : self(self),
      pm(ProjectManager::instance())
{
    MenuManager& mm = ext->menuManager().setPath("/Tools").setPath(_("History"));
    currentMenu = mm.currentMenu();
    currentMenu->setContextMenuPolicy(Qt::CustomContextMenu);
    clearProject = new Action;
    clearProject->setText(_("Clear all histories"));
    clearProject->sigTriggered().connect([&](){ onClearProjectTriggered(); });
    currentMenu->addAction(clearProject);
    currentMenu->addSeparator();
    currentMenu->sigTriggered().connect([&](QAction* action){ onCurrentMenuTriggered(action); });

    contextMenu = new Menu;
    Action* removeProject = new Action;
    removeProject->setText(_("Remove"));
    removeProject->sigTriggered().connect([&](){ onRemoveProjectTriggered(); });
    contextMenu->addAction(removeProject);

    QObject::connect(currentMenu, &Menu::customContextMenuRequested, [=](const QPoint& pos){ onCustomContextMenuRequested(pos); });

    pm->sigProjectLoaded().connect([&](int level){ onProjectLoaded(); });

    Mapping& config = *AppConfig::archive()->openMapping("history");
    if(config.isValid()) {
        restore(config);
    }
}


HistoryManager::~HistoryManager()
{
    delete impl;
}


HistoryManagerImpl::~HistoryManagerImpl()
{
    store(*AppConfig::archive()->openMapping("history"));
}


void HistoryManager::initialize(ExtensionManager* ext)
{
    ext->manage(new HistoryManager(ext));
}


void HistoryManagerImpl::onClearProjectTriggered()
{
    int numHistories = currentMenu->actions().size() - 2;
    for(int i = 0; i < numHistories; ++i) {
        currentMenu->removeAction(currentMenu->actions()[2]);
    }
}


void HistoryManagerImpl::onRemoveProjectTriggered()
{
    currentMenu->removeAction(currentMenu->actionAt(pos));
}


void HistoryManagerImpl::onCurrentMenuTriggered(QAction* action)
{
    Action* triggeredProject = dynamic_cast<Action*>(action);
    if(triggeredProject != clearProject) {
        bool result = pm->tryToCloseProject();
        if(result) {
            pm->clearProject();
            MessageView::instance()->flush();
            pm->loadProject(action->text().toStdString());
        }
    }
}


void HistoryManagerImpl::onCustomContextMenuRequested(const QPoint& pos)
{
    this->pos = pos;
    if(currentMenu->actionAt(this->pos) != clearProject) {
        contextMenu->exec(currentMenu->mapToGlobal(pos));
    }
}


void HistoryManagerImpl::onProjectLoaded()
{
    string filename = pm->currentProjectFile();
    if(!filename.empty()) {
        Action* action = new Action();
        action->setText(filename.c_str());
        currentMenu->addAction(action);

        int maxHistory = 10;
        int numHistories = currentMenu->actions().size();
        if(numHistories > maxHistory) {
            currentMenu->removeAction(currentMenu->actions()[0]);
        }
    }
}


void HistoryManagerImpl::store(Mapping& archive)
{
    int numHistories = currentMenu->actions().size() - 2;
    archive.write("num_histories", numHistories);
    for(int i = 0; i < numHistories; ++i) {
        QAction* action = currentMenu->actions()[i + 2];
        string key = "history_" + to_string(i);
        string filename = action->text().toStdString();
        archive.write(key, filename);
    }
}


void HistoryManagerImpl::restore(const Mapping& archive)
{
    int numHistories = archive.get("num_histories", 0);
    for(int i = 0; i < numHistories; ++i) {
        string key = "history_" + to_string(i);
        string filename = archive.get(key, "");
        Action* action = new Action();
        action->setText(filename.c_str());
        currentMenu->addAction(action);
    }
}
