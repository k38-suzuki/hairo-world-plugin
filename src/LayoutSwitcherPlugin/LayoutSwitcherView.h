/**
   @author Kenta Suzuki
*/

#ifndef CNOID_LAYOUT_SWITCHER_PLUGIN_LAYOUT_SWITCHER_VIEW_H
#define CNOID_LAYOUT_SWITCHER_PLUGIN_LAYOUT_SWITCHER_VIEW_H

#include <cnoid/View>
#include "exportdecl.h"

namespace cnoid {

class LayoutSwitcherViewImpl;

class CNOID_EXPORT LayoutSwitcherView : public View
{
public:
    LayoutSwitcherView();
    virtual ~LayoutSwitcherView();

    static void initializeClass(ExtensionManager* ext);
    static LayoutSwitcherView* instance();

    virtual bool storeState(Archive& archive) override;
    virtual bool restoreState(const Archive& archive) override;

private:
    LayoutSwitcherViewImpl* impl;
    friend class LayoutSwitcherViewImpl;
};

}

#endif
