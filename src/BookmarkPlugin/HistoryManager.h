/**
   \file
   \author Kenta Suzuki
*/

#ifndef CNOID_BOOKMARKPLUGIN_HISTORYMANAGER_H
#define CNOID_BOOKMARKPLUGIN_HISTORYMANAGER_H

#include <cnoid/ExtensionManager>
#include <vector>

namespace cnoid {

class HistoryManagerImpl;

class HistoryManager
{
public:
    HistoryManager();
    virtual ~HistoryManager();

    static void initializeClass(ExtensionManager* ext);
    static void finalizeClass();
    static HistoryManager* instance();

    std::vector<std::string> histories() const;
    bool addHistory(const std::string& history);
    void setMaxHistory(const int& maxHistory);
    int maxHistory() const;
    void clearHistory();

    SignalProxy<void(std::string filename)> sigHistoryAdded();

private:
    HistoryManagerImpl* impl;
    friend class HistoryManagerImpl;
};

}

#endif // CNOID_BOOKMARKPLUGIN_HISTORYMANAGER_H
