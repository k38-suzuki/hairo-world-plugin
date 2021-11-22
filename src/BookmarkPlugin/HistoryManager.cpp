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
    HistoryManager* self;

    ProjectManager* pm;
    MappingPtr config;
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
    Menu* menu = mm.currentMenu();
    menu->sigTriggered().connect([&](QAction* action){
        string filename = action->text().toStdString();
        pm->loadProject(filename);
    });

    config = AppConfig::archive()->openMapping("history");
    int numHistories = config->get("num_history", 0);
    for(int i = 0; i < numHistories; ++i) {
        string key = "history_" + to_string(i);
        string history = config->get(key, "");
        mm.addItem(history);
    }

    pm->sigProjectLoaded().connect([&](int level){
        string filename = pm->currentProjectFile();
        if(!filename.empty()) {
            mm.addItem(filename);

            int maxHistory = 10;
            Menu* menu = mm.currentMenu();
            int numHistories = menu->actions().size();
            if(numHistories > maxHistory) {
                Action* action = dynamic_cast<Action*>(menu->actions()[0]);
                menu->removeAction(action);
            }

            numHistories = menu->actions().size();
            config->write("num_history", numHistories);
            for(int i = 0; i < numHistories; ++i) {
                Action* action = dynamic_cast<Action*>(menu->actions()[i]);
                string key = "history_" + to_string(i);
                string filename = action->text().toStdString();
                config->write(key, filename);
            }
        }
    });
}


HistoryManager::~HistoryManager()
{
    delete impl;
}


void HistoryManager::initialize(ExtensionManager* ext)
{
    ext->manage(new HistoryManager(ext));
}
