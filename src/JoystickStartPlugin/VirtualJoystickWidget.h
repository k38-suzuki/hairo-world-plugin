/**
   @author Kenta Suzuki
*/

#ifndef CNOID_JOYSTICKSTART_PLUGIN_VIRTUAL_JOYSTICK_WIDGET_H
#define CNOID_JOYSTICKSTART_PLUGIN_VIRTUAL_JOYSTICK_WIDGET_H

#include <cnoid/Widget>

namespace cnoid {

class VirtualJoystickWidget : public Widget
{
public:
    VirtualJoystickWidget();
    virtual ~VirtualJoystickWidget();

protected:
    virtual void keyPressEvent(QKeyEvent* event) override;
    virtual void keyReleaseEvent(QKeyEvent* event) override;

private:
    class Impl;
    Impl* impl;
};

}

#endif // CNOID_JOYSTICKSTART_PLUGIN_VIRTUAL_JOYSTICK_WIDGET_H