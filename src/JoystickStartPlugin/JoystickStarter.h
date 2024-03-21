/**
   @author Kenta Suzuki
*/

#ifndef CNOID_JOYSTICK_START_PLUGIN_JOYSTICK_STARTER_H
#define CNOID_JOYSTICK_START_PLUGIN_JOYSTICK_STARTER_H

namespace cnoid {

class ExtensionManager;

class JoystickStarter
{
public:
    JoystickStarter();
    virtual ~JoystickStarter();

    static void initializeClass(ExtensionManager* ext);

private:
    class Impl;
    Impl* impl;
};

}

#endif
