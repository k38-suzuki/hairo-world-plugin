/**
   \file
   \author Kenta Suzuki
*/

#ifndef CNOID_JOYSTICKSTATUSPLUGIN_CONFIGURABLEJOYSTICK_H
#define CNOID_JOYSTICKSTATUSPLUGIN_CONFIGURABLEJOYSTICK_H

#include <cnoid/Joystick>
#include <cnoid/Referenced>
#include "KeyConfigView.h"
#include "exportdecl.h"

namespace cnoid {

class ConfigurableJoystickImpl;

class CNOID_EXPORT ConfigurableJoystick : public Referenced
{
public:
    ConfigurableJoystick() {
        joystick = &defaultJoystick;
        configView = KeyConfigView::instance();
    }

    int numAxes() const {
        return joystick->numAxes();
    }

    int numButtons() const {
        return joystick->numButtons();
    }

    bool readCurrentState() {
        return joystick->readCurrentState();
    }

    double getPosition(int axis) const {
        int index = configView->axisID(axis);
        return joystick->getPosition(index);
    }

    bool getButtonState(int button) const {
        int index = configView->buttonID(button);
        return joystick->getButtonState(index);
    }

private:
    JoystickInterface* joystick;
    Joystick defaultJoystick;
    KeyConfigView* configView;
};

}

#endif // CNOID_JOYSTICKSTATUSPLUGIN_CONFIGURABLEJOYSTICK_H
