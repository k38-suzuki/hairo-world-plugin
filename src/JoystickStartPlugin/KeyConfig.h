/**
   @author Kenta Suzuki
*/

#ifndef CNOID_JOYSTICKSTART_PLUGIN_KEY_CONFIG_H
#define CNOID_JOYSTICKSTART_PLUGIN_KEY_CONFIG_H

namespace cnoid {

class KeyConfig
{
public:
    KeyConfig();
    virtual ~KeyConfig();

    int axisID(const int& axis);
    int buttonID(const int& button);

    void showConfig();

private:
    class Impl;
    Impl* impl;
};

}

#endif // CNOID_JOYSTICKSTART_PLUGIN_KEY_CONFIG_H
