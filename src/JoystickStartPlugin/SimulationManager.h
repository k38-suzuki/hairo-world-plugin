/**
   \file
   \author Kenta Suzuki
*/

#ifndef CNOID_JOYSTICKSTARTPLUGIN_SIMULATIONMANAGER_H
#define CNOID_JOYSTICKSTARTPLUGIN_SIMULATIONMANAGER_H

#include <cnoid/ExtensionManager>

namespace cnoid {

class SimulationManagerImpl;

class SimulationManager
{
public:
    SimulationManager(ExtensionManager* ext);
    virtual ~SimulationManager();

    static void initialize(ExtensionManager* ext);

private:
    SimulationManagerImpl* impl;
    friend class SimulationManagerImpl;
};

}

#endif // CNOID_JOYSTICKSTARTPLUGIN_SIMULATIONMANAGER_H
