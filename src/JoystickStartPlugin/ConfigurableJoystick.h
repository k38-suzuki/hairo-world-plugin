/**
   @author Kenta Suzuki
*/

#ifndef CNOID_JOYSTICK_START_PLUGIN_CONFIGURABLE_JOYSTICK_H
#define CNOID_JOYSTICK_START_PLUGIN_CONFIGURABLE_JOYSTICK_H

#include <cnoid/Joystick>
#include <cnoid/Referenced>
#include "KeyConfig.h"
#include "exportdecl.h"

namespace cnoid {

class ConfigurableJoystickImpl;

class CNOID_EXPORT ConfigurableJoystick : public Referenced
{
public:
    ConfigurableJoystick() {
        joystick = &defaultJoystick;
        configView = new KeyConfig;
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

    void show() {
        configView->showConfig();
    }

private:
    JoystickInterface* joystick;
    Joystick defaultJoystick;
    KeyConfig* configView;
};

}

#endif
