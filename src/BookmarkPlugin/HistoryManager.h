/**
   @author Kenta Suzuki
*/

#ifndef CNOID_BOOKMARK_PLUGIN_HISTORY_MANAGER_H
#define CNOID_BOOKMARK_PLUGIN_HISTORY_MANAGER_H

#include <cnoid/ExtensionManager>

namespace cnoid {

class HistoryManagerImpl;

class HistoryManager
{
public:
    HistoryManager(ExtensionManager* ext);
    virtual ~HistoryManager();

    static void initializeClass(ExtensionManager* ext);

private:
    HistoryManagerImpl* impl;
    friend class HistoryManagerImpl;
};

}

#endif
