/**
   @author Kenta Suzuki
*/

#include <cnoid/Format>
#include <cnoid/Plugin>
#include "CFDSimulatorItem.h"

using namespace cnoid;

class CFDPlugin : public Plugin
{
public:

    CFDPlugin() : Plugin("CFD")
    {
        require("Body");
        require("SimpleCollider");
    }

    virtual bool initialize() override
    {
        CFDSimulatorItem::initializeClass(this);
        return true;
    }

    virtual const char* description() const override
    {
        static std::string text =
            formatC("CFD Plugin Version {}\n", CNOID_FULL_VERSION_STRING) +
            "\n" +
            "Copyright (c) 2020 Japan Atomic Energy Agency.\n"
            "\n" +
            MITLicenseText();
        return text.c_str();
    }
};

CNOID_IMPLEMENT_PLUGIN_ENTRY(CFDPlugin)