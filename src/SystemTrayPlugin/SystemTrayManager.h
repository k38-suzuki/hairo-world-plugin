/**
   \file
   \author Kenta Suzuki
*/

#ifndef CNOID_SYSTEM_TRAY_PLUGIN_SYSTEM_TRAY_MANAGER_H
#define CNOID_SYSTEM_TRAY_PLUGIN_SYSTEM_TRAY_MANAGER_H

#include <cnoid/ExtensionManager>
#include "SystemTrayIcon.h"
#include "exportdecl.h"

namespace cnoid {

class SystemTrayManagerImpl;

class CNOID_EXPORT SystemTrayManager
{
public:
    SystemTrayManager();
    virtual ~SystemTrayManager();

    static void initializeClass(ExtensionManager* ext);

    static SystemTrayIcon* addIcon();
    static SystemTrayIcon* addIcon(const QIcon& icon);

private:
    SystemTrayManagerImpl* impl;
    friend class SystemTrayManagerImpl;
};

}

#endif // CNOID_SYSTEM_TRAY_PLUGIN_SYSTEM_TRAY_MANAGER_H