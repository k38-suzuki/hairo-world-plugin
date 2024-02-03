/**
   @author Kenta Suzuki
*/

#ifndef CNOID_JOYSTICK_START_PLUGIN_SIMPLE_SIMULATION_VIEW_H
#define CNOID_JOYSTICK_START_PLUGIN_SIMPLE_SIMULATION_VIEW_H

#include <cnoid/View>

namespace cnoid {

class SimpleSimulationView : public View
{
public:
    SimpleSimulationView();
    virtual ~SimpleSimulationView();

    static void initializeClass(ExtensionManager* ext);
    static SimpleSimulationView* instance();

private:
    class Impl;
    Impl* impl;
};

}

#endif
