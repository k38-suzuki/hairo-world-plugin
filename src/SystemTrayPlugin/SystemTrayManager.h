/**
   \file
   \author Kenta Suzuki
*/

#ifndef CNOID_SYSTEM_TRAY_MANAGER_H
#define CNOID_SYSTEM_TRAY_MANAGER_H

#include <cnoid/Action>
#include <cnoid/ExtensionManager>
#include <cnoid/Menu>
#include "exportdecl.h"

namespace cnoid {

class SystemTrayManagerImpl;

class CNOID_EXPORT SystemTrayManager
{
public:
    SystemTrayManager();
    virtual ~SystemTrayManager();

    static void initializeClass(ExtensionManager* ext);

    static Menu* addMenu();
    static Menu* addMenu(const QIcon& icon);

private:
    SystemTrayManagerImpl* impl;
    friend class SystemTrayManagerImpl;
};

}

#endif // CNOID_SYSTEM_TRAY_MANAGER_H