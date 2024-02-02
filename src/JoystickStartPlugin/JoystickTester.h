/**
   @author Kenta Suzuki
*/

#ifndef CNOID_JOYSTICK_START_PLUGIN_JOYSTICK_TESTER_H
#define CNOID_JOYSTICK_START_PLUGIN_JOYSTICK_TESTER_H

#include <cnoid/ExtensionManager>

namespace cnoid {

class JoystickTesterImpl;

class JoystickTester
{
public:
    JoystickTester();
    virtual ~JoystickTester();

    static void initializeClass(ExtensionManager* ext);

private:
    JoystickTesterImpl* impl;
    friend class JoystickTesterImpl;
};

}

#endif
