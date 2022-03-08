/**
   \file
   \author Kenta Suzuki
*/

#ifndef CNOID_KIOSKPLUGIN_KIOSKMANAGER_H
#define CNOID_KIOSKPLUGIN_KIOSKMANAGER_H

#include <cnoid/ExtensionManager>

namespace cnoid {

class KIOSKManagerImpl;

class KIOSKManager
{
public:
    KIOSKManager(ExtensionManager* ext);
    virtual ~KIOSKManager();

    static void initialize(ExtensionManager* ext);

private:
    KIOSKManagerImpl* impl;
    friend class KIOSKManagerImpl;
};

}

#endif // CNOID_KIOSKPLUGIN_KIOSKMANAGER_H
