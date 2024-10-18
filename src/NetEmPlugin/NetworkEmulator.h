/**
   @author Kenta Suzuki
*/

#ifndef CNOID_NETEM_PLUGIN_NETWORK_EMULATOR_H
#define CNOID_NETEM_PLUGIN_NETWORK_EMULATOR_H

namespace cnoid {

class ExtensionManager;

class NetworkEmulator
{
public:
    static void initializeClass(ExtensionManager* ext);

    NetworkEmulator();
    virtual ~NetworkEmulator();
};

}

#endif // CNOID_NETEM_PLUGIN_NETWORK_EMULATOR_H