/*! @file
  @author Kenta Suzuki
*/

#include <cnoid/Plugin>
#include <fmt/format.h>
#include "OnScreenJoystickView.h"
#include "JoystickLoggerItem.h"
#include "JoystickStatusView.h"
#include "JoystickTester.h"
#include "SimulationManager.h"
#include "SimpleSimulationView.h"

using namespace cnoid;

namespace {

class JoystickStatusPlugin : public Plugin
{
public:

    JoystickStatusPlugin() : Plugin("JoystickStatus")
    {
        require("Body");
    }

    virtual bool initialize() override
    {
        OnScreenJoystickView::initializeClass(this);
        JoystickLoggerItem::initializeClass(this);
        JoystickStatusView::initializeClass(this);
        JoystickTester::initializeClass(this);
        SimulationManager::initializeClass(this);
        SimpleSimulationView::initializeClass(this);
        return true;
    }

    virtual const char* description() const override
    {
        static std::string text =
            fmt::format("JoystickStatus Plugin Version {}\n", CNOID_FULL_VERSION_STRING) +
            "\n" +
            "Copyrigh (c) 2020 Japan Atomic Energy Agency.\n"
            "\n" +
            MITLicenseText();
        return text.c_str();
    }
};

}

CNOID_IMPLEMENT_PLUGIN_ENTRY(JoystickStatusPlugin)