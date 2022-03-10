/**
   \file
   \author Kenta Suzuki
*/

#ifndef CNOID_KIOSKPLUGIN_KIOSKVIEW_H
#define CNOID_KIOSKPLUGIN_KIOSKVIEW_H

#include <cnoid/View>
#include "BookmarkWidget.h"
#include "LogWidget.h"
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

    BookmarkWidget* bookmarkWidget();
    LogWidget* logWidget();

    virtual bool storeState(Archive& archive) override;
    virtual bool restoreState(const Archive& archive) override;

private:
    KIOSKViewImpl* impl;
    friend class KIOSKViewImpl;
};

}

#endif // CNOID_KIOSKPLUGIN_KIOSKVIEW_H
