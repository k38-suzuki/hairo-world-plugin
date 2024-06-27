/**
   @author Kenta Suzuki
*/

#include <cnoid/Plugin>
#include <fmt/format.h>
#include "IntervalStarterBar.h"
#include "JoystickLoggerItem.h"
#include "OnScreenJoystickView.h"
#include "JoystickTester.h"
#include "JoystickStarter.h"

using namespace cnoid;

class JoystickStartPlugin : public Plugin
{
public:

    JoystickStartPlugin() : Plugin("JoystickStart")
    {
        require("Body");
    }

    virtual bool initialize() override
    {
        IntervalStarterBar::initialize(this);
        JoystickLoggerItem::initializeClass(this);
        OnScreenJoystickView::initializeClass(this);
        JoystickTester::initializeClass(this);
        JoystickStarter::initializeClass(this);
        return true;
    }

    virtual const char* description() const override
    {
        static std::string text =
            fmt::format("JoystickStart Plugin Version {}\n", CNOID_FULL_VERSION_STRING) +
            "\n" +
            "Copyrigh (c) 2020 Japan Atomic Energy Agency.\n"
            "\n" +
            MITLicenseText();
        return text.c_str();
    }
};

CNOID_IMPLEMENT_PLUGIN_ENTRY(JoystickStartPlugin)