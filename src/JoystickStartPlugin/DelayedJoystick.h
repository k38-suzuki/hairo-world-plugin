/**
   @author Kenta Suzuki
*/

#ifndef CNOID_JOYSTICKSTART_PLUGIN_DELAYED_JOYSTICK_H
#define CNOID_JOYSTICKSTART_PLUGIN_DELAYED_JOYSTICK_H

#include <cnoid/Joystick>
#include <cnoid/Referenced>
#include <vector>
#include "exportdecl.h"

namespace cnoid {

class JoystickState : public JoystickInterface
{
public:
    JoystickState() {}

    JoystickState(const Joystick& joystick, bool copyState = false) {
        int num_axes = joystick.numAxes();
        axes.resize(num_axes, 0.0);

        int num_buttons = joystick.numButtons();
        buttons.resize(num_buttons, false);

        if(copyState) {
            for(int i = 0; i < num_axes; ++i) {
                axes[i] = joystick.getPosition(i);
            }

            for(int i = 0; i < num_buttons; ++i) {
                buttons[i] = joystick.getButtonState(i);
            }
        }
    }

    virtual int numAxes() const {
        return axes.size();
    }

    virtual int numButtons() const {
        return buttons.size();
    }

    virtual bool readCurrentState() {
        return true;
    }

    virtual double getPosition(int axis) const {
        return axes[axis];
    }

    virtual bool getButtonState(int button) const {
        return buttons[button];
    }

private:
    std::vector<double> axes;
    std::vector<bool> buttons;
};

class CNOID_EXPORT DelayedJoystick : public Referenced
{
public:

    DelayedJoystick() {
        joystick = &defaultJoystick;
        buffer.clear();
    }

    bool makeReady(const double& timeStep, const double& delay) {
        buffer.resize(delay / 1000.0 / timeStep, JoystickState(defaultJoystick, false));
        return true;
    }

    void setJoystick(JoystickInterface* joystick) {
        this->joystick = joystick;
    }

    bool readCurrentState() {
        joystick->readCurrentState();
        buffer.push_back(JoystickState(defaultJoystick, true));
        currentState = buffer.front();
        buffer.erase(buffer.begin());
        return true;
    }

    double getPosition(int axis, double threshold = 0.0) const {
        // double pos = joystick->getPosition(axis);
        // return (fabs(pos) >= threshold) ? pos : 0.0;
        return currentState.getPosition(axis);
    }

    bool getButtonState(int button) const {
        // return joystick->getButtonState(button);
        return currentState.getButtonState(button);
    }

private:
    JoystickInterface* joystick;
    Joystick defaultJoystick;
    std::vector<JoystickState> buffer;
    JoystickState currentState;
};

typedef ref_ptr<DelayedJoystick> DelayedJoystickPtr;

}

#endif // CNOID_JOYSTICKSTART_PLUGIN_DELAYED_JOYSTICK_H