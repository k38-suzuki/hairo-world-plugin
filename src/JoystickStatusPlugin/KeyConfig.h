/**
   \file
   \author Kenta Suzuki
*/

#ifndef CNOID_JOYSTICKSTATUSPLUGIN_KEYCONFIG_H
#define CNOID_JOYSTICKSTATUSPLUGIN_KEYCONFIG_H

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

#endif // CNOID_JOYSTICKSTATUSPLUGIN_KEYCONFIG_H
