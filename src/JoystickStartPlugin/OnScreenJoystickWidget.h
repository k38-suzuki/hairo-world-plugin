/**
   @author Kenta Suzuki
*/

#ifndef CNOID_JOYSTICKSTART_PLUGIN_ON_SCREEN_JOYSTICK_WIDGET_H
#define CNOID_JOYSTICKSTART_PLUGIN_ON_SCREEN_JOYSTICK_WIDGET_H

#include <QWidget>

namespace cnoid {

class OnScreenJoystickWidget : public QWidget
{
public:
    OnScreenJoystickWidget(QWidget* parent = nullptr);
    virtual ~OnScreenJoystickWidget();

private:
    class Impl;
    Impl* impl;
};

}

#endif // CNOID_JOYSTICKSTART_PLUGIN_ON_SCREEN_JOYSTICK_WIDGET_H
