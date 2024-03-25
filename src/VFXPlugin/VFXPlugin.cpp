/**
   @author Kenta Suzuki
*/

#include <cnoid/Plugin>
#include <fmt/format.h>
#include "VFXColliderItem.h"
#include "VFXVisionSimulatorItem.h"

using namespace cnoid;

class VFXPlugin : public Plugin
{
public:
    VFXPlugin() : Plugin("VFX")
    {
        require("Body");
    }

    virtual bool initialize() override
    {
        VFXColliderItem::initializeClass(this);
        VFXVisionSimulatorItem::initializeClass(this);
        return true;
    }

    virtual const char* description() const override
    {
        static std::string text =
            fmt::format("VFX Plugin Version {}\n", CNOID_FULL_VERSION_STRING) +
            "\n" +
            "Copyright (c) 2024 Japan Atomic Energy Agency.\n"
            "\n" +
            MITLicenseText();
        return text.c_str();
    }
};

CNOID_IMPLEMENT_PLUGIN_ENTRY(VFXPlugin)