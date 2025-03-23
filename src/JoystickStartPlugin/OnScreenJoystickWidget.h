/**
   @author Kenta Suzuki
*/

#ifndef CNOID_JOYSTICKSTART_PLUGIN_ON_SCREEN_JOYSTICK_WIDGET_H
#define CNOID_JOYSTICKSTART_PLUGIN_ON_SCREEN_JOYSTICK_WIDGET_H

#include <cnoid/Signal>
#include <QWidget>

namespace cnoid {

class OnScreenJoystickWidget : public QWidget
{
public:
    OnScreenJoystickWidget(QWidget* parent = nullptr);
    virtual ~OnScreenJoystickWidget();

    SignalProxy<void(int id, double position)> sigAxis();
    SignalProxy<void(int id, bool isPressed)> sigButton();

private:
    class Impl;
    Impl* impl;
};

}

#endif // CNOID_JOYSTICKSTART_PLUGIN_ON_SCREEN_JOYSTICK_WIDGET_H
