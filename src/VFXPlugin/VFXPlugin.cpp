/**
   @author Kenta Suzuki
*/

#include <cnoid/Format>
#include <cnoid/Plugin>
#include "VFXVisionSimulatorItem.h"

using namespace cnoid;

class VFXPlugin : public Plugin
{
public:

    VFXPlugin() : Plugin("VFX")
    {
        require("Body");
        require("GLVisionSimulator");
        require("SimpleCollider");
    }

    virtual bool initialize() override
    {
        VFXVisionSimulatorItem::initializeClass(this);
        return true;
    }

    virtual const char* description() const override
    {
        static std::string text =
            formatC("VFX Plugin Version {}\n", CNOID_FULL_VERSION_STRING) +
            "\n" +
            "Copyright (c) 2024 Japan Atomic Energy Agency.\n"
            "\n" +
            MITLicenseText();
        return text.c_str();
    }
};

CNOID_IMPLEMENT_PLUGIN_ENTRY(VFXPlugin)