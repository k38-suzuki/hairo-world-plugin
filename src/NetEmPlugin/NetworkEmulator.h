/**
   \file
   \author Kenta Suzuki
*/

#ifndef CNOID_NETEMPLUGIN_NETWORKEMULATOR_H
#define CNOID_NETEMPLUGIN_NETWORKEMULATOR_H

#include <cnoid/ExtensionManager>

namespace cnoid {

class NetworkEmulatorImpl;

class NetworkEmulator
{
public:
    NetworkEmulator();
    virtual ~NetworkEmulator();

    static void initializeClass(ExtensionManager* ext);

private:
    NetworkEmulatorImpl* impl;
    friend class NetworkEmulatorImpl;
};

}

#endif // CNOID_NETEMPLUGIN_NETWORKEMULATOR_H
