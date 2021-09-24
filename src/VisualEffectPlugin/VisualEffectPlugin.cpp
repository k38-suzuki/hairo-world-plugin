/**
   \file
   \author Kenta Suzuki
*/

#include <cnoid/Plugin>
#include <fmt/format.h>
#include "VEAreaItem.h"
#include "VisualEffectorItem.h"

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
        VEAreaItem::initializeClass(this);
        VisualEffectorItem::initializeClass(this);
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
