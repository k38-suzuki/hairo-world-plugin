/**
   \file
   \author Kenta Suzuki
*/

#ifndef CNOID_BOOKMARK_PLUGIN_WORLD_LOG_MANAGER_H
#define CNOID_BOOKMARK_PLUGIN_WORLD_LOG_MANAGER_H

#include <cnoid/ExtensionManager>

namespace cnoid {

class WorldLogManagerImpl;

class WorldLogManager
{
public:
    WorldLogManager(ExtensionManager* ext);
    virtual ~WorldLogManager();

    static void initializeClass(ExtensionManager* ext);
    static WorldLogManager* instance();

    void showWorldLogManagerDialog();

private:
    WorldLogManagerImpl* impl;
    friend class WorldLogManagerImpl;
};

}

#endif // CNOID_BOOKMARK_PLUGIN_WORLD_LOG_MANAGER_H