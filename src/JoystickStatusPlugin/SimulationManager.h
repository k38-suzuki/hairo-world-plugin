/**
   \file
   \author Kenta Suzuki
*/

#ifndef CNOID_JOYSTICK_STATUS_PLUGIN_SIMULATION_MANAGER_H
#define CNOID_JOYSTICK_STATUS_PLUGIN_SIMULATION_MANAGER_H

#include <cnoid/ExtensionManager>

namespace cnoid {

class SimulationManagerImpl;

class SimulationManager
{
public:
    SimulationManager();
    virtual ~SimulationManager();

    static void initializeClass(ExtensionManager* ext);

private:
    SimulationManagerImpl* impl;
    friend class SimulationManagerImpl;
};

}

#endif // CNOID_JOYSTICK_STATUS_PLUGIN_SIMULATION_MANAGER_H