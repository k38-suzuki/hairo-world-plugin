/**
   \file
   \author Kenta Suzuki
*/

#include "HistoryManager.h"
#include <cnoid/AppConfig>
#include <cnoid/MenuManager>
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

    ProjectManager* pm;
    Menu* menu;

    void onActionTriggered(const QAction* action);
    void onProjectLoaded();
    bool store(Mapping& archive);
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
    menu = mm.currentMenu();

    menu->sigTriggered().connect([&](QAction* action){ onActionTriggered(action); });
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


void HistoryManagerImpl::onActionTriggered(const QAction* action)
{
    string filename = action->text().toStdString();
    pm->loadProject(filename);
}


void HistoryManagerImpl::onProjectLoaded()
{
    string filename = pm->currentProjectFile();
    if(!filename.empty()) {
        Action* action = new Action();
        action->setText(filename.c_str());
        menu->addAction(action);

        int maxHistory = 10;
        int numHistories = menu->actions().size();
        if(numHistories > maxHistory) {
            Action* action = dynamic_cast<Action*>(menu->actions()[0]);
            menu->removeAction(action);
        }
    }
}


bool HistoryManagerImpl::store(Mapping& archive)
{
    int numHistories = menu->actions().size();
    archive.write("num_history", numHistories);
    for(int i = 0; i < numHistories; ++i) {
        Action* action = dynamic_cast<Action*>(menu->actions()[i]);
        string key = "history_" + to_string(i);
        string filename = action->text().toStdString();
        archive.write(key, filename);
    }
    return true;
}


void HistoryManagerImpl::restore(const Mapping& archive)
{
    int numHistories = archive.get("num_history", 0);
    for(int i = 0; i < numHistories; ++i) {
        string key = "history_" + to_string(i);
        string filename = archive.get(key, "");
        Action* action = new Action();
        action->setText(filename.c_str());
        menu->addAction(action);
    }
}
