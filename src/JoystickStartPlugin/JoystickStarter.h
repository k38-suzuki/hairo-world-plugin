/**
   \file
   \author Kenta Suzuki
*/

#ifndef CNOID_JOYSTICK_START_PLUGIN_JOYSTICK_STARTER_H
#define CNOID_JOYSTICK_START_PLUGIN_JOYSTICK_STARTER_H

#include <cnoid/ExtensionManager>

namespace cnoid {

class JoystickStarterImpl;

class JoystickStarter
{
public:
    JoystickStarter();
    virtual ~JoystickStarter();

    static void initializeClass(ExtensionManager* ext);

private:
    JoystickStarterImpl* impl;
    friend class JoystickStarterImpl;
};

}

#endif // CNOID_JOYSTICK_START_PLUGIN_JOYSTICK_STARTER_H