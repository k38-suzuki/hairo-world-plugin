/**
   \file
   \author Kenta Suzuki
*/

#ifndef CNOID_KIOSK_PLUGIN_SIMPLE_SIMULATION_VIEW_H
#define CNOID_KIOSK_PLUGIN_SIMPLE_SIMULATION_VIEW_H

#include <cnoid/View>

namespace cnoid {

class SimpleSimulationViewImpl;

class SimpleSimulationView : public View
{
public:
    SimpleSimulationView();
    virtual ~SimpleSimulationView();

    static void initializeClass(ExtensionManager* ext);
    static SimpleSimulationView* instance();

private:
    SimpleSimulationViewImpl* impl;
    friend class SimpleSimulationViewImpl;
};

}

#endif // CNOID_KIOSK_PLUGIN_SIMPLE_SIMULATION_VIEW_H