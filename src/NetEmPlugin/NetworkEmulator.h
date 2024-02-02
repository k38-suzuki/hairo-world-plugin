/**
   @author Kenta Suzuki
*/

#ifndef CNOID_NETEM_PLUGIN_NETWORK_EMULATOR_H
#define CNOID_NETEM_PLUGIN_NETWORK_EMULATOR_H

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

#endif
