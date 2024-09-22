/**
   @author Kenta Suzuki
*/

#include <cnoid/Format>
#include <cnoid/Plugin>
#include "MarkerDetectorItem.h"

using namespace cnoid;

class MarkerDetectPlugin : public Plugin
{
public:

    MarkerDetectPlugin() : Plugin("MarkerDetect")
    {
        require("Body");
        require("MotionCapture");
    }

    virtual bool initialize() override
    {
        MarkerDetectorItem::initializeClass(this);
        return true;
    }

    virtual const char* description() const override
    {
        static std::string text =
            formatC("MarkerDetect Plugin Version {}\n", CNOID_FULL_VERSION_STRING) +
            "\n" +
            "Copyright (c) 2022 Japan Atomic Energy Agency.\n"
            "\n" +
            MITLicenseText();
        return text.c_str();
    }
};

CNOID_IMPLEMENT_PLUGIN_ENTRY(MarkerDetectPlugin)