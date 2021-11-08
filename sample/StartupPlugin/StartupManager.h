/**
   \file
   \author Kenta Suzuki
*/

#ifndef CNOID_STARTUPPLUGIN_STARTUPMANAGER_H
#define CNOID_STARTUPPLUGIN_STARTUPMANAGER_H

#include <cnoid/ExtensionManager>

namespace cnoid {

class StartupManagerImpl;

class StartupManager
{
public:
    StartupManager();
    virtual ~StartupManager();

    static void initializeClass(ExtensionManager* ext);

private:
    StartupManagerImpl* impl;
    friend class StartupManagerImpl;
};

}

#endif // CNOID_STARTUPPLUGIN_STARTUPMANAGER_H
