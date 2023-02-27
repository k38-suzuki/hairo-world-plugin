/**
   \file
   \author Kenta Suzuki
*/

#ifndef CNOID_LAYOUT_SWITCHER_PLUGIN_LAYOUT_SWITCHER_H
#define CNOID_LAYOUT_SWITCHER_PLUGIN_LAYOUT_SWITCHER_H

#include <cnoid/Archive>
#include <cnoid/Widget>

namespace cnoid {

class LayoutSwitcherImpl;

class LayoutSwitcher : public Widget
{
public:
    LayoutSwitcher();
    virtual ~LayoutSwitcher();

    bool storeState(Archive& archive);
    bool restoreState(const Archive& archive);

private:
    LayoutSwitcherImpl* impl;
    friend class LayoutSwitcherImpl;
};

}

#endif
