/**
   \file
   \author Kenta Suzuki
*/

#ifndef CNOID_BOOKMARKPLUGIN_HISTORYMANAGER_H
#define CNOID_BOOKMARKPLUGIN_HISTORYMANAGER_H

#include <cnoid/ExtensionManager>

namespace cnoid {

class HistoryManagerImpl;

class HistoryManager
{
public:
    HistoryManager(ExtensionManager* ext);
    virtual ~HistoryManager();

    static void initialize(ExtensionManager* ext);

private:
    HistoryManagerImpl* impl;
    friend class HistoryManagerImpl;
};

}

#endif // CNOID_BOOKMARKPLUGIN_HISTORYMANAGER_H
