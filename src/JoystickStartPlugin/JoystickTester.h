/**
   @author Kenta Suzuki
*/

#ifndef CNOID_JOYSTICKSTART_PLUGIN_JOYSTICK_TESTER_H
#define CNOID_JOYSTICKSTART_PLUGIN_JOYSTICK_TESTER_H

namespace cnoid {

class ExtensionManager;

class JoystickTester
{
public:
    static void initializeClass(ExtensionManager* ext);
    static JoystickTester* instance();

    JoystickTester();
    virtual ~JoystickTester();

private:
    class Impl;
    Impl* impl;
};

}

#endif // CNOID_JOYSTICKSTART_PLUGIN_JOYSTICK_TESTER_H