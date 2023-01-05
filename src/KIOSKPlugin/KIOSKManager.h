/**
   \file
   \author Kenta Suzuki
*/

#ifndef CNOID_KIOSK_PLUGIN_KIOSK_MANAGER_H
#define CNOID_KIOSK_PLUGIN_KIOSK_MANAGER_H

#include <cnoid/ExtensionManager>
#include <cnoid/Signal>

namespace cnoid {

class KIOSKManagerImpl;

class KIOSKManager
{
public:
    KIOSKManager(ExtensionManager* ext);
    virtual ~KIOSKManager();

    static void initializeClass(ExtensionManager* ext);

    static void setLoggingEnabled(const bool& on);

    static SignalProxy<void(bool)> sigLoggingEnabled();

private:
    KIOSKManagerImpl* impl;
    friend class KIOSKManagerImpl;
};

}

#endif // CNOID_KIOSK_PLUGIN_KIOSK_MANAGER_H