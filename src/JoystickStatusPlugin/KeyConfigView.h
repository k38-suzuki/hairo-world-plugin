/**
   \file
   \author Kenta Suzuki
*/

#ifndef CNOID_JOYSTICKSTATUSPLUGIN_KEYCONFIGVIEW_H
#define CNOID_JOYSTICKSTATUSPLUGIN_KEYCONFIGVIEW_H

#include <cnoid/View>
#include "exportdecl.h"

namespace cnoid {

class KeyConfigViewImpl;

class CNOID_EXPORT KeyConfigView : public View
{
public:
    KeyConfigView();
    virtual ~KeyConfigView();

    static void initializeClass(ExtensionManager* ext);
    static KeyConfigView* instance();

    int axisID(const int& axis);
    int buttonID(const int& button);

    virtual bool storeState(Archive& archive);
    virtual bool restoreState(const Archive& archive);

private:
    KeyConfigViewImpl* impl;
    friend class KeyConfigViewImpl;
};

}

#endif // CNOID_JOYSTICKSTATUSPLUGIN_KEYCONFIGVIEW_H
