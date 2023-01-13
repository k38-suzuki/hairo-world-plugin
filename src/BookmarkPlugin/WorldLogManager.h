/**
   \file
   \author Kenta Suzuki
*/

#ifndef CNOID_KIOSK_PLUGIN_WORLD_LOG_MANAGER_H
#define CNOID_KIOSK_PLUGIN_WORLD_LOG_MANAGER_H

#include <cnoid/ExtensionManager>

namespace cnoid {

class WorldLogManagerImpl;

class WorldLogManager
{
public:
    WorldLogManager(ExtensionManager* ext);
    virtual ~WorldLogManager();

    static void initializeClass(ExtensionManager* ext);

    void addItem(const std::string& filename);

    void showWorldLogManger();

private:
    WorldLogManagerImpl* impl;
    friend class WorldLogManagerImpl;
};

}

#endif // CNOID_KIOSK_PLUGIN_WORLD_LOG_MANAGER_H