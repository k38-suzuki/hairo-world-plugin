/**
   @author Kenta Suzuki
*/

#ifndef CNOID_JOYSTICK_START_PLUGIN_JOYSTICK_START_VIEW_H
#define CNOID_JOYSTICK_START_PLUGIN_JOYSTICK_START_VIEW_H

#include <cnoid/View>
#include "exportdecl.h"

namespace cnoid {

class CNOID_EXPORT OnScreenJoystickView : public View
{
public:
    OnScreenJoystickView();
    virtual ~OnScreenJoystickView();

    static void initializeClass(ExtensionManager* ext);
    
protected:
    virtual void keyPressEvent(QKeyEvent* event) override;
    virtual void keyReleaseEvent(QKeyEvent* event) override;
    virtual void onAttachedMenuRequest(MenuManager& menuManager) override;
    virtual bool storeState(Archive& archive) override;
    virtual bool restoreState(const Archive& archive) override;
    
private:
    class Impl;
    Impl* impl;
};

}

#endif
