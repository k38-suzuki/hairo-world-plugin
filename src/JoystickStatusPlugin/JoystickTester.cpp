/**
   \file
   \author Kenta Suzuki
*/

#include "JoystickTester.h"
#include <cnoid/AppConfig>
#include <cnoid/JoystickCapture>
#include <cnoid/MenuManager>
#include <cnoid/MessageView>
#include <cnoid/ValueTree>
#include <fmt/format.h>
#include "gettext.h"

using namespace cnoid;

namespace {

bool isJoystickTestEnabled_ = false;
JoystickCapture joystick;
MessageView* mv;

}

namespace cnoid {

class JoystickTesterImpl
{
public:
  JoystickTesterImpl(JoystickTester* self);
  JoystickTester* self;
};

}


JoystickTester::JoystickTester()
{
    impl = new JoystickTesterImpl(this);
}


JoystickTesterImpl::JoystickTesterImpl(JoystickTester* self)
    : self(self)
{

}


JoystickTester::~JoystickTester()
{
    delete impl;
}


void JoystickTester::initializeClass(ExtensionManager* ext)
{
    auto jsConfig = AppConfig::archive()->openMapping("joystick_test");

    isJoystickTestEnabled_ = jsConfig->get("js_test", false);

    MenuManager& mm = ext->menuManager().setPath("/" N_("Options"));
    mm.setPath(N_("Joystick"));
    auto jsMenu = mm.currentMenu();

    auto jsTestItem = mm.addCheckItem(_("JoystickTest"));
    jsMenu->sigAboutToShow().connect(
        [jsTestItem](){ jsTestItem->setChecked(isJoystickTestEnabled_); });
    jsTestItem->sigToggled().connect([](bool on){
        isJoystickTestEnabled_ = on;
        AppConfig::archive()->openMapping("joystick_test")->write("js_test", on);
    });

    mv = MessageView::instance();

    joystick.setDevice("/dev/input/js0");

    joystick.sigButton().connect(
        [&](int id, bool isPressed){
            if(isJoystickTestEnabled_) {
                mv->putln(fmt::format("Joystick button {0}: {1}", id, isPressed));
            }
        });

    joystick.sigAxis().connect(
        [&](int id, double position){
            if(isJoystickTestEnabled_) {
                mv->putln(fmt::format("Joystick axis {0}: {1}", id, position));
            }
        });
}
