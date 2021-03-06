/*! @file
  @author Kenta Suzuki
*/

#include <cnoid/Plugin>
#include <fmt/format.h>
#include "JoystickLoggerItem.h"
#include "JoystickStatusView.h"

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
        JoystickLoggerItem::initializeClass(this);
        JoystickStatusView::initializeClass(this);
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
