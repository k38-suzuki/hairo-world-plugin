/**
   \file
   \author Kenta Suzuki
*/

#ifndef CNOID_BOOKMARK_PLUGIN_KIOSK_MANAGER_H
#define CNOID_BOOKMARK_PLUGIN_KIOSK_MANAGER_H

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

private:
    KIOSKManagerImpl* impl;
    friend class KIOSKManagerImpl;
};

}

#endif // CNOID_BOOKMARK_PLUGIN_KIOSK_MANAGER_H