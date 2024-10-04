/**
   @author Kenta Suzuki
*/

#include <cnoid/Format>
#include <cnoid/Plugin>
#include "WRSUtilBar.h"

using namespace cnoid;

class WRSUtilPlugin : public Plugin
{
public:

    WRSUtilPlugin() : Plugin("WRSUtil")
    {
        require("Body");
    }

    virtual bool initialize() override
    {
        WRSUtilBar::initialize(this);
        return true;
    }

    virtual const char* description() const override
    {
        static std::string text =
            formatC("WRSUtil Plugin Version {}\n", CNOID_FULL_VERSION_STRING) +
            "\n" +
            "Copyright (c) 2024 Japan Atomic Energy Agency.\n"
            "\n" +
            MITLicenseText();
        return text.c_str();
    }
};

CNOID_IMPLEMENT_PLUGIN_ENTRY(WRSUtilPlugin)