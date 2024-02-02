/**
   @author Kenta Suzuki
*/

#include <cnoid/Plugin>
#include <fmt/format.h>
#include "CFDSimulatorItem.h"
#include "FluidAreaItem.h"

using namespace cnoid;

namespace {

class CFDPlugin : public Plugin
{
public:

    CFDPlugin() : Plugin("CFD")
    {
        require("Body");
    }

    virtual bool initialize() override
    {
        AreaItem::initializeClass(this);
        FluidAreaItem::initializeClass(this);
        CFDSimulatorItem::initializeClass(this);
        return true;
    }

    virtual const char* description() const override
    {
        static std::string text =
            fmt::format("CFD Plugin Version {}\n", CNOID_FULL_VERSION_STRING) +
            "\n" +
            "Copyright (c) 2020 Japan Atomic Energy Agency.\n"
            "\n" +
            MITLicenseText();
        return text.c_str();
    }
};

}

CNOID_IMPLEMENT_PLUGIN_ENTRY(CFDPlugin)