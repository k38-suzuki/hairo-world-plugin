/*!
  @file
  @author Kenta Suzuki
*/

#ifndef CNOID_JOYSTICKSTATUSPLUGIN_JOYSTICKSTATUSVIEW_H
#define CNOID_JOYSTICKSTATUSPLUGIN_JOYSTICKSTATUSVIEW_H

#include <cnoid/View>
#include "exportdecl.h"

namespace cnoid {

class JoystickStatusViewImpl;

class CNOID_EXPORT JoystickStatusView : public View
{
public:
    JoystickStatusView();
    virtual ~JoystickStatusView();

    static void initializeClass(ExtensionManager* ext);
    
protected:
    virtual void keyPressEvent(QKeyEvent* event);
    virtual void keyReleaseEvent(QKeyEvent* event);
    
    virtual bool storeState(Archive& archive);
    virtual bool restoreState(const Archive& archive);
    
private:
    JoystickStatusViewImpl* impl;
    friend class JoystickStatusViewImpl;
};

}

#endif
