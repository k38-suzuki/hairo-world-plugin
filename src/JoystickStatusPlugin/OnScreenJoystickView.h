/**
   \file
   \author Kenta Suzuki
*/

#ifndef CNOID_JOYSTICKSTATUSPLUGIN_ONSCREENJOYSTICKVIEW_H
#define CNOID_JOYSTICKSTATUSPLUGIN_ONSCREENJOYSTICKVIEW_H

#include <cnoid/View>

namespace cnoid {

class OnScreenJoystickViewImpl;

class OnScreenJoystickView : public View
{
public:
    OnScreenJoystickView();
    virtual ~OnScreenJoystickView();

    static void initializeClass(ExtensionManager* ext);

protected:
    virtual bool storeState(Archive& archive);
    virtual bool restoreState(const Archive& archive);

private:
    OnScreenJoystickViewImpl* impl;
    friend class OnScreenJoystickViewImpl;
};

}

#endif // CNOID_JOYSTICKSTATUSPLUGIN_ONSCREENJOYSTICKVIEW_H
