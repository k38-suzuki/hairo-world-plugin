/**
   \file
   \author Kenta Suzuki
*/

#include <cnoid/Plugin>
#include <fmt/format.h>
#include "CameraVisualizerItem.h"

using namespace cnoid;

namespace {

class ImageEffectPlugin : public Plugin
{
public:

    ImageEffectPlugin() : Plugin("ImageEffect")
    {
        require("Body");
    }

    virtual bool initialize() override
    {
        CameraVisualizerItem::initializeClass(this);
        return true;
    }

    virtual const char* description() const override
    {
        static std::string text =
            fmt::format("ImageEffect Plugin Version {}\n", CNOID_FULL_VERSION_STRING) +
            "\n" +
            "Copyrigh (c) 2019 Japan Atomic Energy Agency.\n"
            "\n" +
            MITLicenseText();
        return text.c_str();
    }
};

}

CNOID_IMPLEMENT_PLUGIN_ENTRY(ImageEffectPlugin)
