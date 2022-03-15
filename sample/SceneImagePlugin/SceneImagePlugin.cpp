/**
   \file
   \author Kenta Suzuki
*/

#include <cnoid/Plugin>
#include <fmt/format.h>
#include "SceneImageView.h"

using namespace cnoid;

namespace {

class SceneImagePlugin : public Plugin
{
public:

    SceneImagePlugin() : Plugin("SceneImage")
    {
        require("Body");
    }

    virtual bool initialize()
    {
        SceneImageView::initializeClass(this);
        return true;
    }

    virtual const char* description() const override
    {
        static std::string text =
            fmt::format("SceneImage Plugin Version {}\n", CNOID_FULL_VERSION_STRING) +
            "\n" +
            "Copyright (c) 2022 Japan Atomic Energy Agency.\n"
            "\n" +
            MITLicenseText();
        return text.c_str();
    }
};

}

CNOID_IMPLEMENT_PLUGIN_ENTRY(SceneImagePlugin)
