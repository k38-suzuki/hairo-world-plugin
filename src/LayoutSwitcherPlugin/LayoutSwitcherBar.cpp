/**
   @author Kenta Suzuki
*/

#include "LayoutSwitcherBar.h"
#include "LayoutSwitcher.h"
#include "gettext.h"

using namespace cnoid;
using namespace std;

namespace cnoid {

class LayoutSwitcherBarImpl
{
public:
    LayoutSwitcherBarImpl(LayoutSwitcherBar* self);
    LayoutSwitcherBar* self;

    LayoutSwitcher* layoutSwitcher;

    bool storeState(Archive& archive);
    bool restoreState(const Archive& archive);
};

}


LayoutSwitcherBar::LayoutSwitcherBar()
    : ToolBar(N_("LayoutSwitcherBar"))
{
    impl = new LayoutSwitcherBarImpl(this);
}


LayoutSwitcherBarImpl::LayoutSwitcherBarImpl(LayoutSwitcherBar* self)
    : self(self)
{
    layoutSwitcher = new LayoutSwitcher;
    self->addWidget(layoutSwitcher);
}


LayoutSwitcherBar::~LayoutSwitcherBar()
{
    delete impl;
}


void LayoutSwitcherBar::initialize(ExtensionManager* ext)
{
    static bool initialized = false;
    if(!initialized) {
        ext->addToolBar(instance());
        initialized  = true;
    }
}


LayoutSwitcherBar* LayoutSwitcherBar::instance()
{
    static LayoutSwitcherBar* switcherBar = new LayoutSwitcherBar;
    return switcherBar;
}


bool LayoutSwitcherBar::storeState(Archive& archive)
{
    return impl->storeState(archive);
}


bool LayoutSwitcherBarImpl::storeState(Archive& archive)
{
    layoutSwitcher->storeState(archive);
    return true;
}


bool LayoutSwitcherBar::restoreState(const Archive& archive)
{
    return impl->restoreState(archive);
}


bool LayoutSwitcherBarImpl::restoreState(const Archive& archive)
{
    layoutSwitcher->restoreState(archive);
    return true;
}
