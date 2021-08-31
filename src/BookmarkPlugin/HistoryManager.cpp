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

HistoryManager* historyManager = nullptr;

namespace cnoid {

class HistoryManagerImpl
{
public:
    HistoryManagerImpl(HistoryManager* self);
    virtual ~HistoryManagerImpl();
    HistoryManager* self;

    vector<string> histories;
    int maxHistory;
    ProjectManager* pm;

    Signal<void(string filename)> sigHistoryAdded;

    bool addHistory(const string& history);
};

}


HistoryManager::HistoryManager()
{
    impl = new HistoryManagerImpl(this);
}


HistoryManagerImpl::HistoryManagerImpl(HistoryManager *self)
    : self(self)
{
    histories.clear();
    maxHistory = 10;
    pm = ProjectManager::instance();

    Mapping* config = AppConfig::archive()->openMapping("History");
    int size = config->get("numHistory", 0);
    for(int i = 0; i < size; ++i) {
        string key = "history" + to_string(i);
        string project = config->get(key, "");
        addHistory(project);
    }
}


HistoryManager::~HistoryManager()
{
    delete impl;
}


HistoryManagerImpl::~HistoryManagerImpl()
{
    int size = histories.size();
    Mapping* config = AppConfig::archive()->openMapping("History");
    config->write("numHistory", size);
    for(int i = 0; i < size; ++i) {
        string project = histories[i];
        string key = "history" + to_string(i);
        config->write(key, project);
    }
}


void HistoryManager::initializeClass(ExtensionManager* ext)
{
    if(!historyManager) {
        historyManager = ext->manage(new HistoryManager());
    }

    MenuManager& mm = ext->menuManager().setPath("/Tools").setPath(_("History"));
}


HistoryManager* HistoryManager::instance()
{
    return historyManager;
}


vector<string> HistoryManager::histories() const
{
    return impl->histories;
}


bool HistoryManager::addHistory(const string& history)
{
    return impl->addHistory(history);
}


bool HistoryManagerImpl::addHistory(const string& history)
{
    if(!history.empty()) {
        histories.push_back(history);
        sigHistoryAdded(history);
        if(histories.size() > maxHistory) {
            histories.erase(histories.begin());
        }
    }
    return true;
}


void HistoryManager::setMaxHistory(const int& maxHistory)
{
    impl->maxHistory = maxHistory;
}


int HistoryManager::maxHistory() const
{
    return impl->maxHistory;
}


void HistoryManager::clearHistory()
{
    impl->histories.clear();
}


SignalProxy<void(string filename)> HistoryManager::sigHistoryAdded()
{
    return impl->sigHistoryAdded;
}
