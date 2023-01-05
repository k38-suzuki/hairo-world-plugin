/**
   \file
   \author Kenta Suzuki
*/

#ifndef CNOID_JOYSTICK_STATUS_PLUGIN_KEY_CONFIG_H
#define CNOID_JOYSTICK_STATUS_PLUGIN_KEY_CONFIG_H

namespace cnoid {

class KeyConfigImpl;

class KeyConfig
{
public:
    KeyConfig();
    virtual ~KeyConfig();

    int axisID(const int& axis);
    int buttonID(const int& button);

    void showConfig();

private:
    KeyConfigImpl* impl;
    friend class KeyConfigImpl;
};

}

#endif // CNOID_JOYSTICK_STATUS_PLUGIN_KEY_CONFIG_H