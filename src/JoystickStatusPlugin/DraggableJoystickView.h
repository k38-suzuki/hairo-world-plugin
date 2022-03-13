/**
   \file
   \author Kenta Suzuki
*/

#ifndef CNOID_JOYSTICKSTATUSPLUGIN_DRAGGABLEJOYSTICKVIEW_H
#define CNOID_JOYSTICKSTATUSPLUGIN_DRAGGABLEJOYSTICKVIEW_H

#include <cnoid/View>

namespace cnoid {

class DraggableJoystickViewImpl;

class DraggableJoystickView : public View
{
public:
    DraggableJoystickView();
    virtual ~DraggableJoystickView();

    static void initializeClass(ExtensionManager* ext);

protected:
    virtual bool storeState(Archive& archive);
    virtual bool restoreState(const Archive& archive);

private:
    DraggableJoystickViewImpl* impl;
    friend class DraggableJoystickViewImpl;
};

}

#endif // CNOID_JOYSTICKSTATUSPLUGIN_DRAGGABLEJOYSTICKVIEW_H
