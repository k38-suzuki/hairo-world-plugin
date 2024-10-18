/**
   \file
   \author  Kenta Suzuki
*/

#include "OnScreenJoystickView.h"
#include <cnoid/MenuManager>
#include <cnoid/ViewManager>
#include <QBoxLayout>
#include <QStackedWidget>
#include "VirtualJoystickWidget.h"
#include "OnScreenJoystickWidget.h"
#include "gettext.h"

using namespace std;
using namespace cnoid;

namespace cnoid {

class OnScreenJoystickView::Impl
{
public:
    OnScreenJoystickView* self;

    Impl(OnScreenJoystickView* self);

    bool isOnScreenJoystickEnabled;
    QStackedWidget* topWidget;
};

}


void OnScreenJoystickView::initializeClass(ExtensionManager* ext)
{
    ext->viewManager().registerClass<OnScreenJoystickView>(
        "OnScreenJoystickView", N_("Virtual Joystick2"), ViewManager::SINGLE_OPTIONAL);
}


OnScreenJoystickView::OnScreenJoystickView()
{
    impl = new Impl(this);
}


OnScreenJoystickView::Impl::Impl(OnScreenJoystickView* self)
    : self(self),
      isOnScreenJoystickEnabled(false)
{
    self->setDefaultLayoutArea(View::BottomCenterArea);

    topWidget = new QStackedWidget;
    topWidget->addWidget(new VirtualJoystickWidget);
    // topWidget->addWidget(new OnScreenJoystickWidget);
    auto vbox = new QVBoxLayout;
    vbox->addWidget(topWidget);
    self->setLayout(vbox);
}


OnScreenJoystickView::~OnScreenJoystickView()
{
    delete impl;
}


void OnScreenJoystickView::onAttachedMenuRequest(MenuManager& menuManager)
{
    // auto screenCheck = menuManager.addCheckItem(_("On-screen Joystick"));
    // screenCheck->setChecked(impl->isOnScreenJoystickEnabled);
    // screenCheck->sigToggled().connect([&](bool checked){
    //     impl->isOnScreenJoystickEnabled = checked;
    //     impl->topWidget->setCurrentIndex(checked ? 1 : 0);
    // });
}


bool OnScreenJoystickView::storeState(Archive& archive)
{
    return true;
}


bool OnScreenJoystickView::restoreState(const Archive& archive)
{
    return true;
}
