/**
   @author Kenta Suzuki
*/

#ifndef CNOID_BOOKMARK_PLUGIN_HISTORY_MANAGER_H
#define CNOID_BOOKMARK_PLUGIN_HISTORY_MANAGER_H

namespace cnoid {

class ExtensionManager;

class HistoryManager
{
public:
    HistoryManager(ExtensionManager* ext);
    virtual ~HistoryManager();

    static void initializeClass(ExtensionManager* ext);

private:
    class Impl;
    Impl* impl;
};

}

#endif // CNOID_BOOKMARK_PLUGIN_HISTORY_MANAGER_H
