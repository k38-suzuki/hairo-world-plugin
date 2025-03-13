/**
    @author Kenta Suzuki
*/

#include <cnoid/Format>
#include <cnoid/Plugin>
#include "GainTestView.h"

using namespace cnoid;

class GainTestPlugin : public Plugin
{
public:

    GainTestPlugin() : Plugin("GainTest")
    {
        require("Body");
        require("Bookmark");
    }

    virtual bool initialize() override
    {
        GainTestView::initializeClass(this);
        return true;
    }

    virtual const char* description() const override
    {
        static std::string text =
            formatC("GainTest Plugin Version {}\n", CNOID_FULL_VERSION_STRING) +
            "\n" +
            "Copyright (c) 2025 Japan Atomic Energy Agency.\n"
            "\n" +
            MITLicenseText();
        return text.c_str();
    }
};

CNOID_IMPLEMENT_PLUGIN_ENTRY(GainTestPlugin)