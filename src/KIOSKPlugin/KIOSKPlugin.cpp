/**
   \file
   \author Kenta Suzuki
*/

#include <cnoid/Plugin>
#include <fmt/format.h>
#include "KIOSKManager.h"
#include "KIOSKView.h"
#include "SimpleSimulationView.h"

using namespace cnoid;

namespace {

class KIOSKPlugin : public Plugin
{
public:

    KIOSKPlugin() : Plugin("KIOSK")
    {
        require("Body");
    }

    virtual bool initialize() override
    {
        KIOSKManager::initialize(this);
        KIOSKView::initializeClass(this);
        SimpleSimulationView::initializeClass(this);
        return true;
    }

    virtual const char* description() const override
    {
        static std::string text =
            fmt::format("KIOSK Plugin Version {}\n", CNOID_FULL_VERSION_STRING) +
            "\n" +
            "Copyright (c) 2022 Japan Atomic Energy Agency.\n"
            "\n" +
            MITLicenseText();
        return text.c_str();
    }
};

}

CNOID_IMPLEMENT_PLUGIN_ENTRY(KIOSKPlugin)
