/**
    @author Kenta Suzuki
*/

#include <cnoid/Format>
#include <cnoid/Plugin>

using namespace cnoid;

class GooglePlugin : public Plugin
{
public:

    GooglePlugin() : Plugin("Google")
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
            formatC("Google Plugin Version {}\n", CNOID_FULL_VERSION_STRING) +
            "\n" +
            "Copyright (c) 2024 Japan Atomic Energy Agency.\n"
            "\n" +
            MITLicenseText();
        return text.c_str();
    }
};

CNOID_IMPLEMENT_PLUGIN_ENTRY(GooglePlugin)