/**
   @author Kenta Suzuki
*/

#ifndef CNOID_BOOKMARK_PLUGIN_KIOSK_MANAGER_H
#define CNOID_BOOKMARK_PLUGIN_KIOSK_MANAGER_H

#include <cnoid/JoystickCapture>

namespace cnoid {

class ExtensionManager;
class SimulatorItem;
class JoyKey;

class KIOSKManager
{
public:
    static void initializeClass(ExtensionManager* ext);

    KIOSKManager(ExtensionManager* ext);
    virtual ~KIOSKManager();

private:
    void onHideMenuBarToggled(const bool& on);
    void onButton(const int& id, const bool& isPressed);

    JoystickCapture joystick;
    SimulatorItem* simulatorItem;
    JoyKey* key;
};

}

#endif // CNOID_BOOKMARK_PLUGIN_KIOSK_MANAGER_H