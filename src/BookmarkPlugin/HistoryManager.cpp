/**
   \file
   \author Kenta Suzuki
*/

#include "HistoryManager.h"
#include <cnoid/AppConfig>
#include <cnoid/MenuManager>
#include <cnoid/ProjectManager>
#include "BookmarkManagerView.h"
#include "gettext.h"

using namespace cnoid;
using namespace std;

HistoryManager* historyManager = nullptr;
MenuManager menuManager;

namespace {

void onProjectLoaded(const string filename)
{
    if(!filename.empty()) {
        Action* historyItem = menuManager.addItem(filename);
        historyManager->addHistory(filename);

        QWidget* current = menuManager.current();
        QMenu* menu = dynamic_cast<QMenu*>(current);
        if(menu) {
            int size = menu->actions().size();
            int max = historyManager->maxHistory();
            if(size > max) {
                Action* action = dynamic_cast<Action*>(menu->actions()[0]);
                menu->removeAction(action);
            }
        }

        historyItem->sigTriggered().connect([&, historyItem]() {
            string history = historyItem->text().toStdString();
            bool on = BookmarkManagerView::openDialogToLoadProject(history);
            if(!on) {
                return;
            }
        });
    }
}

}


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
    int size = config->get("num_history", 0);
    for(int i = 0; i < size; ++i) {
        string key = "history_" + to_string(i);
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
    config->write("num_history", size);
    for(int i = 0; i < size; ++i) {
        string project = histories[i];
        string key = "history_" + to_string(i);
        config->write(key, project);
    }
}


void HistoryManager::initializeClass(ExtensionManager* ext)
{
    if(!historyManager) {
        historyManager = ext->manage(new HistoryManager());
    }

    menuManager = ext->menuManager().setPath("/Tools").setPath(_("History"));
    std::vector<std::string> histories = historyManager->histories();
    for(int i = 0; i < histories.size(); ++i) {
        onProjectLoaded(histories[i]);
    }

    ProjectManager* pm = ProjectManager::instance();
    pm->sigProjectLoaded().connect([&, pm](int level){ onProjectLoaded(pm->currentProjectFile()); });
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
