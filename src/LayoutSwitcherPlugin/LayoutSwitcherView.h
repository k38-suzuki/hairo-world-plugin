/**
   @author Kenta Suzuki
*/

#ifndef CNOID_LAYOUT_SWITCHER_PLUGIN_LAYOUT_SWITCHER_VIEW_H
#define CNOID_LAYOUT_SWITCHER_PLUGIN_LAYOUT_SWITCHER_VIEW_H

#include <cnoid/ToolBar>
#include <cnoid/View>
#include "exportdecl.h"

namespace cnoid {

class CNOID_EXPORT LayoutSwitcherView : public View
{
public:
    static void initializeClass(ExtensionManager* ext);
    static LayoutSwitcherView* instance();

    LayoutSwitcherView();
    virtual ~LayoutSwitcherView();

    virtual bool storeState(Archive& archive) override;
    virtual bool restoreState(const Archive& archive) override;

private:
    class Impl;
    Impl* impl;
};

class CNOID_EXPORT LayoutSwitcherBar : public ToolBar
{
public:
    static void initialize(ExtensionManager* ext);
    static LayoutSwitcherBar* instance();

protected:
    virtual bool storeState(Archive& archive);
    virtual bool restoreState(const Archive& archive);

private:
    LayoutSwitcherBar();
    virtual ~LayoutSwitcherBar();

    class Impl;
    Impl* impl;
};

}

#endif
