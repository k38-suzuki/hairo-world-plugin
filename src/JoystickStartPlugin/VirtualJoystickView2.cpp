/**
   \file
   \author  Kenta Suzuki
*/

#include "VirtualJoystickView2.h"
#include <cnoid/Archive>
#include <cnoid/MenuManager>
#include <cnoid/ViewManager>
#include <QBoxLayout>
#include "VirtualJoystickWidget.h"
#include "gettext.h"

using namespace std;
using namespace cnoid;

namespace cnoid {

class VirtualJoystickView2::Impl
{
public:
    VirtualJoystickView2* self;

    Impl(VirtualJoystickView2* self);

    VirtualJoystickWidget* joystickWidget;
    bool isDraggerViewEnabled;
};

}


void VirtualJoystickView2::initializeClass(ExtensionManager* ext)
{
    ext->viewManager().registerClass<VirtualJoystickView2>(
        "VirtualJoystickView2", N_("Virtual Joystick2"), ViewManager::SINGLE_OPTIONAL);
}


VirtualJoystickView2::VirtualJoystickView2()
{
    impl = new Impl(this);
}


VirtualJoystickView2::Impl::Impl(VirtualJoystickView2* self)
    : self(self),
      isDraggerViewEnabled(false)
{
    self->setDefaultLayoutArea(View::BottomCenterArea);

    joystickWidget = new VirtualJoystickWidget;
    auto vbox = new QVBoxLayout;
    vbox->addWidget(joystickWidget);
    self->setLayout(vbox);
}


VirtualJoystickView2::~VirtualJoystickView2()
{
    delete impl;
}


void VirtualJoystickView2::onAttachedMenuRequest(MenuManager& menuManager)
{
    auto screenCheck = menuManager.addCheckItem(_("Dragger mode"));
    screenCheck->setChecked(impl->isDraggerViewEnabled);
    screenCheck->sigToggled().connect([&](bool checked){
        impl->isDraggerViewEnabled = checked;
        impl->joystickWidget->setViewMode(checked ? ViewMode::DraggerView : ViewMode::NormalView);
    });
}


bool VirtualJoystickView2::storeState(Archive& archive)
{
    archive.write("enable_dragger_mode", impl->isDraggerViewEnabled);
    return true;
}


bool VirtualJoystickView2::restoreState(const Archive& archive)
{
    if(archive.read("enable_dragger_mode", impl->isDraggerViewEnabled)) {
        impl->joystickWidget->setViewMode(
            impl->isDraggerViewEnabled ? ViewMode::DraggerView : ViewMode::NormalView);
    }
    return true;
}