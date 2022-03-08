/**
   \file
   \author Kenta Suzuki
*/

#ifndef CNOID_KIOSKPLUGIN_SIMPLESIMULATIONVIEW_H
#define CNOID_KIOSKPLUGIN_SIMPLESIMULATIONVIEW_H

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

#endif // CNOID_KIOSKPLUGIN_SIMPLESIMULATIONVIEW_H
