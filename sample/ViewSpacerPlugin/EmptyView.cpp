/**
   \file
   \author Kenta Suzuki
*/

#include "EmptyView.h"
#include <cnoid/ViewManager>
#include "gettext.h"

using namespace cnoid;

namespace cnoid {

class EmptyViewImpl
{
public:
    EmptyViewImpl(EmptyView* self);
    EmptyView* self;

    void onActivated();
    void onDeactivated();
};

}


EmptyView::EmptyView()
{
    impl = new EmptyViewImpl(this);
}


EmptyViewImpl::EmptyViewImpl(EmptyView* self)
    : self(self)
{
    self->setDefaultLayoutArea(View::CENTER);
}


EmptyView::~EmptyView()
{
    delete impl;
}


void EmptyView::initializeClass(ExtensionManager* ext)
{
    ext->viewManager().registerClass<EmptyView>(
        "EmptyView", N_("Empty"), ViewManager::MULTI_OPTIONAL);
}


void EmptyView::onActivated()
{
    impl->onActivated();
}


void EmptyViewImpl::onActivated()
{

}


void EmptyView::onDeactivated()
{
    impl->onDeactivated();
}


void EmptyViewImpl::onDeactivated()
{

}
