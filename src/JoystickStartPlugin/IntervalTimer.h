/**
   @author Kenta Suzuki
*/

#ifndef CNOID_JOYSTICKSTART_PLUGIN_INTERVAL_TIMER_H
#define CNOID_JOYSTICKSTART_PLUGIN_INTERVAL_TIMER_H

namespace cnoid {

class ExtensionManager;

class IntervalTimer
{
public:
    static void initializeClass(ExtensionManager* ext);

    IntervalTimer();
    virtual ~IntervalTimer();
};

}

#endif // CNOID_JOYSTICKSTART_PLUGIN_INTERVAL_TIMER_H