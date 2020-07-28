/**
   \file
   \author Kenta Suzuki
*/

#include <cnoid/Plugin>
#include <fmt/format.h>
#include "MarkerPointItem.h"
#include "MotionCaptureSimulatorItem.h"

using namespace cnoid;

namespace {

class MotionCapturePlugin : public Plugin
{
public:

    MotionCapturePlugin() : Plugin("MotionCapture")
    {
        require("Body");
    }

    virtual bool initialize() override
    {
        MarkerPointItem::initializeClass(this);
        MotionCaptureSimulatorItem::initializeClass(this);
        return true;
    }

    virtual const char* description() const override
    {
        static std::string text =
            fmt::format("MotionCapture Plugin Version {}\n", CNOID_FULL_VERSION_STRING) +
            "\n" +
            "Copyrigh (c) 2019 Japan Atomic Energy Agency.\n"
            "\n" +
            MITLicenseText();
        return text.c_str();
    }
};

}

CNOID_IMPLEMENT_PLUGIN_ENTRY(MotionCapturePlugin)
