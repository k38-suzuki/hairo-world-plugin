/**
   @author Kenta Suzuki
*/

#include <cnoid/Format>
#include <cnoid/Plugin>
#include "MinIOClient.h"
#include "ObjectBrowser.h"

using namespace cnoid;

class MinIOClientPlugin : public Plugin
{
public:

    MinIOClientPlugin() : Plugin("MinIOClient")
    {
        require("Body");
    }

    virtual bool initialize() override
    {
        MinIOClient::initializeClass(this);
        ObjectBrowser::initializeClass(this);
        return true;
    }

    virtual const char* description() const override
    {
        static std::string text =
            formatC("MinIOClient Plugin Version {}\n", CNOID_FULL_VERSION_STRING) +
            "\n" +
            "Copyright (c) 2025 Japan Atomic Energy Agency.\n"
            "\n" +
            MITLicenseText();
        return text.c_str();
    }
};

CNOID_IMPLEMENT_PLUGIN_ENTRY(MinIOClientPlugin)