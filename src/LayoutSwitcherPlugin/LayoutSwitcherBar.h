/**
   \file
   \author Kenta Suzuki
*/

#ifndef CNOID_LAYOUT_SWITCHER_PLUGIN_LAYOUT_SWITCHER_BAR_H
#define CNOID_LAYOUT_SWITCHER_PLUGIN_LAYOUT_SWITCHER_BAR_H

#include <cnoid/ExtensionManager>
#include <cnoid/ToolBar>
#include "exportdecl.h"

namespace cnoid {

class LayoutSwitcherBarImpl;

class CNOID_EXPORT LayoutSwitcherBar : public ToolBar
{
public:
    LayoutSwitcherBar();
    virtual ~LayoutSwitcherBar();

    static void initialize(ExtensionManager* ext);
    static LayoutSwitcherBar* instance();

protected:
    virtual bool storeState(Archive& archive);
    virtual bool restoreState(const Archive& archive);

private:
    LayoutSwitcherBarImpl* impl;
    friend class LayoutSwitcherBarImpl;
};

}

#endif // CNOID_LAYOUT_SWITCHER_PLUGIN_LAYOUT_SWITCHER_BAR_H