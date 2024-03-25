/**
   @author Kenta Suzuki
*/

#include <cnoid/Plugin>
#include <fmt/format.h>
#include "SimpleSetupItem.h"

using namespace cnoid;

class OMPLPlugin : public Plugin
{
public:
    OMPLPlugin() : Plugin("OMPL")
    {
        require("Body");
    }

    virtual bool initialize() override
    {
        SimpleSetupItem::initializeClass(this);
        return true;
    }

    virtual const char* description() const override
    {
        static std::string text =
            fmt::format("OMPL Plugin Version {}\n", CNOID_FULL_VERSION_STRING) +
            "\n" +
            "Copyrigh (c) 2020 Japan Atomic Energy Agency.\n"
            "\n" +
            MITLicenseText();
        return text.c_str();
    }
};

CNOID_IMPLEMENT_PLUGIN_ENTRY(OMPLPlugin)