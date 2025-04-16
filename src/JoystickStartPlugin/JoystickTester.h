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

    JoystickTester();
    virtual ~JoystickTester();
};

} // namespace cnoid

#endif // CNOID_JOYSTICKSTART_PLUGIN_JOYSTICK_TESTER_H