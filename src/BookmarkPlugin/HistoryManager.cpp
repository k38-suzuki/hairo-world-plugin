/**
   \file
   \author Kenta Suzuki
*/

#include "HistoryManager.h"
#include <cnoid/AppConfig>
#include <cnoid/MenuManager>
#include <cnoid/ProjectManager>
#include <vector>
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

    ExtensionManager* ext;
    int maxHistory;
    vector<string> histories;
    ProjectManager* pm;
    MappingPtr config;

    void onProjectLoaded(const string& filename);
};

}


HistoryManager::HistoryManager(ExtensionManager* ext)
{
    impl = new HistoryManagerImpl(this, ext);
}


HistoryManagerImpl::HistoryManagerImpl(HistoryManager* self, ExtensionManager* ext)
    : self(self),
      pm(ProjectManager::instance()),
      ext(ext)
{
    maxHistory = 10;
    histories.clear();

    config = AppConfig::archive()->openMapping("history");
    int numHistories = config->get("num_history", 0);
    for(int i = 0; i < numHistories; ++i) {
        string key = "history_" + to_string(i);
        string history = config->get(key, "");
        onProjectLoaded(history);
    }

    pm->sigProjectLoaded().connect([&](int level){ onProjectLoaded(pm->currentProjectFile()); });
}


HistoryManager::~HistoryManager()
{
    delete impl;
}


HistoryManagerImpl::~HistoryManagerImpl()
{
    int numHistories = histories.size();
    config->write("num_history", numHistories);
    for(int i = 0; i < numHistories; ++i) {
        string key = "history_" + to_string(i);
        string history = histories[i];
        config->write(key, history);
    }
}


void HistoryManager::initialize(ExtensionManager* ext)
{
    HistoryManager* manager = ext->manage(new HistoryManager(ext));

    MenuManager& mm = ext->menuManager().setPath("/Tools").setPath(_("History"));
}


void HistoryManagerImpl::onProjectLoaded(const string& filename)
{
    MenuManager& mm = ext->menuManager().setPath("/Tools").setPath(_("History"));

    if(!filename.empty()) {
        Action* historyItem = mm.addItem(filename);
        historyItem->sigTriggered().connect([=](){ pm->loadProject(filename); });

        histories.push_back(filename);
        if(histories.size() > maxHistory) {
            histories.erase(histories.begin());
        }

        QWidget* current = mm.current();
        QMenu* menu = dynamic_cast<QMenu*>(current);
        if(menu) {
            int numHistories = menu->actions().size();
            if(numHistories > maxHistory) {
                Action* action = dynamic_cast<Action*>(menu->actions()[0]);
                menu->removeAction(action);
            }
        }
    }
}
