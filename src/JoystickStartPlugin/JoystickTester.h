/**
   @author Kenta Suzuki
*/

#ifndef CNOID_JOYSTICK_START_PLUGIN_JOYSTICK_TESTER_H
#define CNOID_JOYSTICK_START_PLUGIN_JOYSTICK_TESTER_H


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

#endif
