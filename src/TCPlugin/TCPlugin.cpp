/**
   \file
   \author Kenta Suzuki
*/


#include <cnoid/Plugin>
#include <fmt/format.h>
#include "TCAreaItem.h"
#include "TCSimulatorItem.h"

using namespace cnoid;

namespace {

class TCPlugin : public Plugin
{
public:

    TCPlugin() : Plugin("TC")
    {
        require("Body");
    }

    virtual bool initialize() override
    {
        TCAreaItem::initializeClass(this);
        TCSimulatorItem::initializeClass(this);
        return true;
    }

    virtual const char* description() const override
    {
        static std::string text =
            fmt::format("TC Plugin Version {}\n", CNOID_FULL_VERSION_STRING) +
            "\n" +
            "Copyright (c) 2020 Japan Atomic Energy Agency.\n"
            "\n" +
            MITLicenseText();
        return text.c_str();
    }
};

}

CNOID_IMPLEMENT_PLUGIN_ENTRY(TCPlugin)
