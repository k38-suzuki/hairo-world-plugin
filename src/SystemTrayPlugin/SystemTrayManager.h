/**
   @author Kenta Suzuki
*/

#ifndef CNOID_SYSTEMTRAY_PLUGIN_SYSTEM_TRAY_MANAGER_H
#define CNOID_SYSTEMTRAY_PLUGIN_SYSTEM_TRAY_MANAGER_H

#include "SystemTrayIcon.h"
#include "exportdecl.h"

namespace cnoid {

class ExtensionManager;

class CNOID_EXPORT SystemTrayManager
{
public:
    static void initializeClass(ExtensionManager* ext);

    SystemTrayManager();
    virtual ~SystemTrayManager();

    static SystemTrayIcon* addIcon();
    static SystemTrayIcon* addIcon(const QIcon& icon);

private:
    class Impl;
    Impl* impl;
};

}

#endif // CNOID_SYSTEMTRAY_PLUGIN_SYSTEM_TRAY_MANAGER_H
