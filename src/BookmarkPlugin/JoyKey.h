/**
   @author Kenta Suzuki
*/

#ifndef CNOID_BOOKMARK_PLUGIN_JOY_KEY_H
#define CNOID_BOOKMARK_PLUGIN_JOY_KEY_H

#include <cnoid/Signal>

namespace cnoid {

class JoyKey
{
public:
    JoyKey(bool useDefaultKey = false);
    virtual ~JoyKey();

    enum InputID {
        DIRECTIONAL_PAD_H_AXIS_LEFT,
        DIRECTIONAL_PAD_H_AXIS_RIGHT,
        DIRECTIONAL_PAD_V_AXIS_UP,
        DIRECTIONAL_PAD_V_AXIS_DOWN,
        L_TRIGGER_AXIS,
        R_TRIGGER_AXIS,
        A_BUTTON,
        B_BUTTON,
        X_BUTTON,
        Y_BUTTON,
        L_BUTTON,
        R_BUTTON,
        SELECT_BUTTON,
        START_BUTTON,
        L_STICK_BUTTON,
        R_STICK_BUTTON,
        LOGO_BUTTON,
        NUM_INPUTS
    };

    void addInput(const int& inputID);
    void clear();

    SignalProxy<void()> sigLocked();
    SignalProxy<void()> sigUnlocked();

private:
    class Impl;
    Impl* impl;
};

}

#endif
