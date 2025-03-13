/**
   @author Kenta Suzuki
*/

#include "JoyKey.h"
#include <cnoid/Joystick>
#include <cnoid/JoystickCapture>
#include <vector>

using namespace std;
using namespace cnoid;

namespace {

struct JoystickInfo {
    int id;
    bool isAxis;
    double position;
};

JoystickInfo joystickInfo[] = {
    { Joystick::DIRECTIONAL_PAD_H_AXIS,  true, -1.0 },
    { Joystick::DIRECTIONAL_PAD_H_AXIS,  true,  1.0 },
    { Joystick::DIRECTIONAL_PAD_V_AXIS,  true, -1.0 },
    { Joystick::DIRECTIONAL_PAD_V_AXIS,  true,  1.0 },
    {         Joystick::L_TRIGGER_AXIS,  true, -1.0 },
    {         Joystick::R_TRIGGER_AXIS,  true, -1.0 },
    {               Joystick::A_BUTTON, false,  0.0 },
    {               Joystick::B_BUTTON, false,  0.0 },
    {               Joystick::X_BUTTON, false,  0.0 },
    {               Joystick::Y_BUTTON, false,  0.0 },
    {               Joystick::L_BUTTON, false,  0.0 },
    {               Joystick::R_BUTTON, false,  0.0 },
    {          Joystick::SELECT_BUTTON, false,  0.0 },
    {           Joystick::START_BUTTON, false,  0.0 },
    {         Joystick::L_STICK_BUTTON, false,  0.0 },
    {         Joystick::R_STICK_BUTTON, false,  0.0 },
    {            Joystick::LOGO_BUTTON, false,  0.0 }
};

const int defaultKey[] = {
    JoyKey::DIRECTIONAL_PAD_V_AXIS_UP,
    JoyKey::DIRECTIONAL_PAD_V_AXIS_UP,
    JoyKey::DIRECTIONAL_PAD_V_AXIS_DOWN,
    JoyKey::DIRECTIONAL_PAD_V_AXIS_DOWN,
    JoyKey::DIRECTIONAL_PAD_H_AXIS_LEFT,
    JoyKey::DIRECTIONAL_PAD_H_AXIS_RIGHT,
    JoyKey::DIRECTIONAL_PAD_H_AXIS_LEFT,
    JoyKey::DIRECTIONAL_PAD_H_AXIS_RIGHT,
    JoyKey::A_BUTTON,
    JoyKey::B_BUTTON
};

}

namespace cnoid {

class JoyKey::Impl
{
public:
    JoyKey* self;

    Impl(JoyKey* self, bool useDefaultKey);

    JoystickCapture joystick;
    int count;
    vector<JoystickInfo> key;

    Signal<void()> sigLocked;
    Signal<void()> sigUnlocked;

    void onAxis(const int& id, const double& position);
    void onButton(const int& id, bool isPressed);
    void tryToUnlock();
};

}


JoyKey::JoyKey(bool useDefaultKey)
{
    impl = new Impl(this, useDefaultKey);
}


JoyKey::Impl::Impl(JoyKey* self, bool useDefaultKey)
    : self(self)
{
    count = 0;
    key.clear();
    if(useDefaultKey) {
        for(int i = 0; i < 10; ++i) {
            int inputID = defaultKey[i];
            JoystickInfo& info = joystickInfo[inputID];
            key.push_back(info);
        }
    }

    joystick.setDevice("/dev/input/js0");
    joystick.sigAxis().connect([&](int id, double position){ onAxis(id, position); });
    joystick.sigButton().connect([&](int id, bool isPressed){ onButton(id, isPressed); });
}


JoyKey::~JoyKey()
{
    delete impl;
}


void JoyKey::addInput(const int& inputID)
{
    JoystickInfo& info = joystickInfo[inputID];
    impl->key.push_back(info);
}


void JoyKey::clear()
{
    impl->count = 0;
    impl->key.clear();
}


SignalProxy<void()> JoyKey::sigLocked()
{
    return impl->sigLocked;
}


SignalProxy<void()> JoyKey::sigUnlocked()
{
    return impl->sigUnlocked;
}


void JoyKey::Impl::onAxis(const int& id, const double& position)
{
    if(fabs(position) > 0.0) {
        JoystickInfo& info = key[count];
        if(info.isAxis) {
            if((int)position == (int)info.position) {
                if(id == info.id) {
                    ++count;
                    tryToUnlock();
                } else {
                    count = 0;
                    sigLocked();
                }
            }
        }
    }
}


void JoyKey::Impl::onButton(const int& id, bool isPressed)
{
    if(isPressed) {
        JoystickInfo& info = key[count];
        if(!info.isAxis) {
            if(id == info.id) {
                ++count;
                tryToUnlock();
            } else {
                count = 0;
                sigLocked();
            }
        }
    }
}


void JoyKey::Impl::tryToUnlock()
{
    if(count == key.size()) {
        count = 0;
        sigUnlocked();
    }
}