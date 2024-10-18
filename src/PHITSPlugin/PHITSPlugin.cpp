/**
   @author Kenta Suzuki
*/

#include <cnoid/Format>
#include <cnoid/Plugin>
#include "CrossSectionItem.h"
#include "DoseSimulatorItem.h"
#include "GammaImagerItem.h"
#include "GammaVisionSimulatorItem.h"

using namespace cnoid;

class PHITSPlugin : public Plugin
{
public:

    PHITSPlugin() : Plugin("PHITS")
    {
        require("Body");
        require("VFX");
    }

    virtual bool initialize() override
    {
        CrossSectionItem::initializeClass(this);
        DoseSimulatorItem::initializeClass(this);
        GammaImagerItem::initializeClass(this);
        // GammaVisionSimulatorItem::initializeClass(this);
        return true;
    }

    virtual const char* description() const override
    {
        static std::string text =
            formatC("PHITS Plugin Version {}\n", CNOID_FULL_VERSION_STRING) +
            "\n" +
            "Copyrigh (c) 2021 Japan Atomic Energy Agency.\n"
            "\n" +
            MITLicenseText();
        return text.c_str();
    }
};

CNOID_IMPLEMENT_PLUGIN_ENTRY(PHITSPlugin)