/**
   @author Kenta Suzuki
*/

#include <cnoid/Format>
#include <cnoid/Plugin>
#include "IntervalTimer.h"
#include "JoystickLoggerItem.h"
#include "OnScreenJoystickView.h"
#include "JoystickTester.h"
#include "JoystickStarter.h"
#include "DigitalClock.h"

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
        IntervalTimer::initializeClass(this);
        JoystickLoggerItem::initializeClass(this);
        // OnScreenJoystickView::initializeClass(this);
        JoystickTester::initializeClass(this);
        JoystickStarter::initializeClass(this);
        DigitalClock::initializeClass(this);
        return true;
    }

    virtual const char* description() const override
    {
        static std::string text =
            formatC("JoystickStart Plugin Version {}\n", CNOID_FULL_VERSION_STRING) +
            "\n" +
            "Copyrigh (c) 2020 Japan Atomic Energy Agency.\n"
            "\n" +
            MITLicenseText();
        return text.c_str();
    }
};

CNOID_IMPLEMENT_PLUGIN_ENTRY(JoystickStartPlugin)