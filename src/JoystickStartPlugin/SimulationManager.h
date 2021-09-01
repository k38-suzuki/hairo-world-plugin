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
    SimulationManager();
    virtual ~SimulationManager();

    static void initializeClass(ExtensionManager* ext);

private:
    SimulationManagerImpl* impl;
    friend class SimulationManagerImpl;
};

}

#endif // CNOID_JOYSTICKSTARTPLUGIN_SIMULATIONMANAGER_H
