/**
   @author Kenta Suzuki
*/

#include <cnoid/Plugin>
#include <fmt/format.h>

using namespace cnoid;

class ZBarPlugin : public Plugin
{
public:
    ZBarPlugin() : Plugin("ZBar")
    {
        require("Body");
    }
    
    virtual bool initialize() override
    {
        return true;
    }

    virtual const char* description() const override
    {
        static std::string text =
            fmt::format("ZBar Plugin Version {}\n", CNOID_FULL_VERSION_STRING) +
            "\n" +
            "Copyright (c) 2024 Japan Atomic Energy Agency.\n"
            "\n" +
            MITLicenseText();
        return text.c_str();
    }
};

CNOID_IMPLEMENT_PLUGIN_ENTRY(ZBarPlugin)