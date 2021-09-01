/**
   \file
   \author Kenta Suzuki
*/

#include <cnoid/Plugin>
#include <fmt/format.h>
#include "FluidAreaItem.h"
#include "FluidDynamicsSimulatorItem.h"

using namespace cnoid;

namespace {

class FluidDynamicsPlugin : public Plugin
{
public:

    FluidDynamicsPlugin() : Plugin("FluidDynamics")
    {
        require("Body");
    }

    virtual bool initialize() override
    {
        AreaItem::initializeClass(this);
        FluidAreaItem::initializeClass(this);
        FluidDynamicsSimulatorItem::initializeClass(this);
        return true;
    }

    virtual const char* description() const override
    {
        static std::string text =
            fmt::format("FluidDynamics Plugin Version {}\n", CNOID_FULL_VERSION_STRING) +
            "\n" +
            "Copyright (c) 2020 Japan Atomic Energy Agency.\n"
            "\n" +
            MITLicenseText();
        return text.c_str();
    }
};

}

CNOID_IMPLEMENT_PLUGIN_ENTRY(FluidDynamicsPlugin)
