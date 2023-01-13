/**
   \file
   \author Kenta Suzuki
*/

#ifndef CNOID_KIOSK_PLUGIN_KIOSK_VIEW_H
#define CNOID_KIOSK_PLUGIN_KIOSK_VIEW_H

#include <cnoid/View>
#include "exportdecl.h"

namespace cnoid {

class KIOSKViewImpl;

class CNOID_EXPORT KIOSKView : public View
{
public:
    KIOSKView();
    virtual ~KIOSKView();

    static void initializeClass(ExtensionManager* ext);
    static KIOSKView* instance();

    virtual bool storeState(Archive& archive) override;
    virtual bool restoreState(const Archive& archive) override;

protected:
    virtual void keyPressEvent(QKeyEvent* event) override;

private:
    KIOSKViewImpl* impl;
    friend class KIOSKViewImpl;
};

}

#endif // CNOID_KIOSK_PLUGIN_KIOSK_VIEW_H