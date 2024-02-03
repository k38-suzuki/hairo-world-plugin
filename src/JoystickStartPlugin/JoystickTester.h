/**
   @author Kenta Suzuki
*/

#ifndef CNOID_JOYSTICK_START_PLUGIN_JOYSTICK_TESTER_H
#define CNOID_JOYSTICK_START_PLUGIN_JOYSTICK_TESTER_H

#include <cnoid/ExtensionManager>

namespace cnoid {

class JoystickTester
{
public:
    JoystickTester();
    virtual ~JoystickTester();

    static void initializeClass(ExtensionManager* ext);

private:
    class Impl;
    Impl* impl;
};

}

#endif
