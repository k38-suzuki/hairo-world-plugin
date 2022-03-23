/**
   \file
   \author Kenta Suzuki
*/

#include <cnoid/Plugin>
#include <fmt/format.h>
#include "InertiaCalculator.h"

using namespace cnoid;

namespace {

class InertiaCalculatorPlugin : public Plugin
{
public:

    InertiaCalculatorPlugin() : Plugin("InertiaCalculator")
    {
        require("Body");
    }

    virtual bool initialize() override
    {
        InertiaCalculator::initializeClass(this);
        return true;
    }

    virtual const char* description() const override
    {
        static std::string text =
            fmt::format("InertiaCalculator Plugin Version {}\n", CNOID_FULL_VERSION_STRING) +
            "\n" +
            "Copyright (c) 2020 Japan Atomic Energy Agency.\n"
            "\n" +
            MITLicenseText();
        return text.c_str();
    }
};

}

CNOID_IMPLEMENT_PLUGIN_ENTRY(InertiaCalculatorPlugin)
