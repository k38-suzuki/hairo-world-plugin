/**
   @author Kenta Suzuki
*/

#ifndef CNOID_BOOKMARK_PLUGIN_WORLD_LOG_MANAGER_H
#define CNOID_BOOKMARK_PLUGIN_WORLD_LOG_MANAGER_H

namespace cnoid {

class ExtensionManager;

class WorldLogManager
{
public:
    static void initializeClass(ExtensionManager* ext);
    static WorldLogManager* instance();

    WorldLogManager();
    virtual ~WorldLogManager();

    void show();

private:
    class Impl;
    Impl* impl;
};

}

#endif
