/**
   @author Kenta Suzuki
*/

#ifndef CNOID_JOYSTICK_START_PLUGIN_ON_SCREEN_JOYSTICK_WIDGET_H
#define CNOID_JOYSTICK_START_PLUGIN_ON_SCREEN_JOYSTICK_WIDGET_H

#include <cnoid/Widget>

namespace cnoid {

class OnScreenJoystickWidget : public Widget
{
public:
    OnScreenJoystickWidget();
    virtual ~OnScreenJoystickWidget();

private:
    class Impl;
    Impl* impl;
};

}

#endif
