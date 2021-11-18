/**
   \file
   \author Kenta Suzuki
*/

#ifndef CNOID_JOYSTICKSTARTPLUGIN_SIMULATIONMANAGER_H
#define CNOID_JOYSTICKSTARTPLUGIN_SIMULATIONMANAGER_H

#include <cnoid/ExtensionManager>
#include "exportdecl.h"

namespace cnoid {

class SimulationManagerImpl;

class CNOID_EXPORT SimulationManager
{
public:
    SimulationManager();
    virtual ~SimulationManager();

    static void initialize(ExtensionManager* ext);
    static SimulationManager* instance();

    void start();
    void restart();
    void stop();
    void pause();

private:
    SimulationManagerImpl* impl;
    friend class SimulationManagerImpl;
};

}

#endif // CNOID_JOYSTICKSTARTPLUGIN_SIMULATIONMANAGER_H
