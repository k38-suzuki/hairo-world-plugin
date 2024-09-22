/**
   @author Kenta Suzuki
*/

#include <cnoid/Format>
#include <cnoid/Plugin>
#include "QRReader.h"

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
        QRReader::initializeClass(this);
        return true;
    }

    virtual const char* description() const override
    {
        static std::string text =
            formatC("ZBar Plugin Version {}\n", CNOID_FULL_VERSION_STRING) +
            "\n" +
            "Copyright (c) 2024 Japan Atomic Energy Agency.\n"
            "\n" +
            MITLicenseText();
        return text.c_str();
    }
};

CNOID_IMPLEMENT_PLUGIN_ENTRY(ZBarPlugin)