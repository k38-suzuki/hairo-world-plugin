/**
   @author Kenta Suzuki
*/

#include <cnoid/Format>
#include <cnoid/Plugin>
#include "BeepItem.h"

using namespace cnoid;

class BeepPlugin : public Plugin
{
public:
    BeepPlugin() : Plugin("Beep")
    {
        require("Body");
    }

    virtual bool initialize() override
    {
        BeepItem::initializeClass(this);
        return true;
    }

    virtual const char* description() const override
    {
        static std::string text =
            formatC("Beep Plugin Version {}\n", CNOID_FULL_VERSION_STRING) +
            "\n" +
            "Copyright (c) 2022 Japan Atomic Energy Agency.\n"
            "\n" +
            MITLicenseText();
        return text.c_str();
    }
};

CNOID_IMPLEMENT_PLUGIN_ENTRY(BeepPlugin)