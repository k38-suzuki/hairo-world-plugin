/**
   @author Kenta Suzuki
*/

#include <cnoid/Plugin>
#include <fmt/format.h>
#include "NetworkEmulator.h"
#include "TCColliderItem.h"
#include "NetworkEmulatorItem.h"

using namespace cnoid;

class NetEmPlugin : public Plugin
{
public:
    NetEmPlugin() : Plugin("NetEm")
    {
        require("Body");
    }

    virtual bool initialize() override
    {
        NetworkEmulator::initializeClass(this);
        TCColliderItem::initializeClass(this);
        NetworkEmulatorItem::initializeClass(this);
        return true;
    }

    virtual const char* description() const override
    {
        static std::string text =
            fmt::format("NetEm Plugin Version {}\n", CNOID_FULL_VERSION_STRING) +
            "\n" +
            "Copyright (c) 2020 Japan Atomic Energy Agency.\n"
            "\n" +
            MITLicenseText();
        return text.c_str();
    }
};

CNOID_IMPLEMENT_PLUGIN_ENTRY(NetEmPlugin)