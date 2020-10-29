/**
   \file
   \author Kenta Suzuki
*/

#include <cnoid/Plugin>
#include <fmt/format.h>
#include "BoxTerrainBuilderDialog.h"

using namespace cnoid;

namespace {

class BoxTerrainBuilderPlugin : public Plugin
{
public:

    BoxTerrainBuilderPlugin() : Plugin("BoxTerrainBuilder")
    {
        require("Body");
    }

    virtual bool initialize() override
    {
        BoxTerrainBuilderDialog::initialzeClass(this);
        return true;
    }

    virtual const char* description() const override
    {
        static std::string text =
            fmt::format("BoxTerrainBuilder Plugin Version {}\n", CNOID_FULL_VERSION_STRING) +
            "\n" +
            "Copyright (c) 2020 Japan Atomic Energy Agency.\n"
            "\n" +
            MITLicenseText();
        return text.c_str();
    }
};

}

CNOID_IMPLEMENT_PLUGIN_ENTRY(BoxTerrainBuilderPlugin)
