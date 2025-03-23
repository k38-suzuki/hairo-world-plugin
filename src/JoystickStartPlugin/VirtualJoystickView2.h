/**
   @author Kenta Suzuki
*/

#ifndef CNOID_JOYSTICKSTART_PLUGIN_VIRTUAL_JOYSTICK_VIEW2_H
#define CNOID_JOYSTICKSTART_PLUGIN_VIRTUAL_JOYSTICK_VIEW2_H

#include <cnoid/View>
#include "exportdecl.h"

namespace cnoid {

class CNOID_EXPORT VirtualJoystickView2 : public View
{
public:
    static void initializeClass(ExtensionManager* ext);

    VirtualJoystickView2();
    virtual ~VirtualJoystickView2();

protected:
    virtual void onAttachedMenuRequest(MenuManager& menuManager) override;
    virtual bool storeState(Archive& archive) override;
    virtual bool restoreState(const Archive& archive) override;

private:
    class Impl;
    Impl* impl;
};

}

#endif // CNOID_JOYSTICKSTART_PLUGIN_VIRTUAL_JOYSTICK_VIEW2_H