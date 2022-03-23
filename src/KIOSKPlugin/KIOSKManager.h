/**
   \file
   \author Kenta Suzuki
*/

#ifndef CNOID_KIOSKPLUGIN_KIOSKMANAGER_H
#define CNOID_KIOSKPLUGIN_KIOSKMANAGER_H

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

#endif // CNOID_KIOSKPLUGIN_KIOSKMANAGER_H
