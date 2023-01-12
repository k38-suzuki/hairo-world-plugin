/**
   \file
   \author Kenta Suzuki
*/

#include <cnoid/Plugin>
#include <fmt/format.h>
#include "SystemTrayManager.h"

using namespace cnoid;

namespace {

class SystemTrayPlugin : public Plugin
{
public:

    SystemTrayPlugin() : Plugin("SystemTray")
    {
        require("Body");
    }

    virtual bool initialize() override
    {
        SystemTrayManager::initializeClass(this);
        return true;
    }

    virtual const char* description() const override
    {
        static std::string text =
            fmt::format("SystemTray Plugin Version {}\n", CNOID_FULL_VERSION_STRING) +
            "\n" +
            "Copyright (c) 2023 Japan Atomic Energy Agency.\n"
            "\n" +
            MITLicenseText();
        return text.c_str();
    }
};

}

CNOID_IMPLEMENT_PLUGIN_ENTRY(SystemTrayPlugin)