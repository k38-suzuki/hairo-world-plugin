/**
   \file
   \author Kenta Suzuki
*/

#ifndef CNOID_JOYSTICK_START_PLUGIN_ON_SCREEN_JOYSTICK_WIDGET_H
#define CNOID_JOYSTICK_START_PLUGIN_ON_SCREEN_JOYSTICK_WIDGET_H

#include <cnoid/Widget>

namespace cnoid {

class OnScreenJoystickWidgetImpl;

class OnScreenJoystickWidget : public Widget
{
public:
    OnScreenJoystickWidget();
    virtual ~OnScreenJoystickWidget();

private:
    OnScreenJoystickWidgetImpl* impl;
    friend class OnScreenJoystickWidgetImpl;
};

}

#endif // CNOID_JOYSTICK_START_PLUGIN_ON_SCREEN_JOYSTICK_WIDGET_H