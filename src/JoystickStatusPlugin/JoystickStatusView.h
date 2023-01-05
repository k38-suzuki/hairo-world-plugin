/*!
  @file
  @author Kenta Suzuki
*/

#ifndef CNOID_JOYSTICK_STATUS_PLUGIN_JOYSTICK_STATUS_VIEW_H
#define CNOID_JOYSTICK_STATUS_PLUGIN_JOYSTICK_STATUS_VIEW_H

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
    virtual void keyPressEvent(QKeyEvent* event) override;
    virtual void keyReleaseEvent(QKeyEvent* event) override;
    virtual void onAttachedMenuRequest(MenuManager& menuManager) override;
    virtual bool storeState(Archive& archive) override;
    virtual bool restoreState(const Archive& archive) override;
    
private:
    JoystickStatusViewImpl* impl;
    friend class JoystickStatusViewImpl;
};

}

#endif // CNOID_JOYSTICK_STATUS_PLUGIN_JOYSTICK_STATUS_VIEW_H