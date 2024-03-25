/**
   @author Kenta Suzuki
*/

#ifndef CNOID_NETEMPLUGIN_NETWORK_EMULATOR_H
#define CNOID_NETEMPLUGIN_NETWORK_EMULATOR_H

namespace cnoid {

class ExtensionManager;

class NetworkEmulator
{
public:
    static void initializeClass(ExtensionManager* ext);

    NetworkEmulator();
    virtual ~NetworkEmulator();

private:
    class Impl;
    Impl* impl;
};

}

#endif
