/**
   @author Kenta Suzuki
*/

#ifndef CNOID_JOYSTICKSTART_PLUGIN_ONSCREEN_JOYSTICK_VIEW_H
#define CNOID_JOYSTICKSTART_PLUGIN_ONSCREEN_JOYSTICK_VIEW_H

#include <cnoid/View>
#include "exportdecl.h"

namespace cnoid {

class CNOID_EXPORT OnScreenJoystickView : public View
{
public:
    static void initializeClass(ExtensionManager* ext);

    OnScreenJoystickView();
    virtual ~OnScreenJoystickView();

protected:
    virtual void onAttachedMenuRequest(MenuManager& menuManager) override;
    virtual bool storeState(Archive& archive) override;
    virtual bool restoreState(const Archive& archive) override;

private:
    class Impl;
    Impl* impl;
};

}

#endif // CNOID_JOYSTICKSTART_PLUGIN_ONSCREEN_JOYSTICK_VIEW_H
