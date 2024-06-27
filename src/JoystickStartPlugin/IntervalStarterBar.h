/**
   @author Kenta Suzuki
*/

#ifndef CNOID_JOYSTICKSTART_PLUGIN_INTERVAL_STARTER_BAR_H
#define CNOID_JOYSTICKSTART_PLUGIN_INTERVAL_STARTER_BAR_H

#include <cnoid/ToolBar>
#include "exportdecl.h"

namespace cnoid {

class ExtensionManager;

class CNOID_EXPORT IntervalStarterBar : public ToolBar
{
public:
    static void initialize(ExtensionManager* ext);
    static IntervalStarterBar* instance();

private:
    class Impl;
    Impl* impl;

    IntervalStarterBar();
    ~IntervalStarterBar();
};

}

#endif // CNOID_JOYSTICKSTART_PLUGIN_INTERVAL_STARTER_BAR_H