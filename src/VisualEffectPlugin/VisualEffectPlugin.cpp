/**
   \file
   \author Kenta Suzuki
*/

#include <cnoid/Plugin>
#include <fmt/format.h>
#include "CameraVisualizerItem.h"
#include "VisualEffectDialog.h"

using namespace cnoid;

namespace {

class VisualEffectPlugin : public Plugin
{
public:

    VisualEffectPlugin() : Plugin("VisualEffect")
    {
        require("Body");
    }

    virtual bool initialize() override
    {
        VisualEffectDialog::initializeClass(this);
        CameraVisualizerItem::initializeClass(this);
        return true;
    }

    virtual const char* description() const override
    {
        static std::string text =
            fmt::format("VisualEffect Plugin Version {}\n", CNOID_FULL_VERSION_STRING) +
            "\n" +
            "Copyright (c) 2020 Japan Atomic Energy Agency.\n"
            "\n" +
            MITLicenseText();
        return text.c_str();
    }
};

}

CNOID_IMPLEMENT_PLUGIN_ENTRY(VisualEffectPlugin)
