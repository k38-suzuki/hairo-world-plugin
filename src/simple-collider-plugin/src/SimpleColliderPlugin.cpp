/**
   @author Kenta Suzuki
*/

#include <cnoid/Plugin>
#include <fmt/format.h>
#include "SimpleColliderItem.h"

using namespace cnoid;

namespace {

class SimpleColliderPlugin : public Plugin
{
public:
    
    SimpleColliderPlugin() : Plugin("SimpleCollider")
    {
        require("Body");
    }
    
    virtual bool initialize()
    {
        SimpleColliderItem::initializeClass(this);
        return true;
    }

    virtual const char* description() const override
    {
        static std::string text =
            fmt::format("SimpleCollider Plugin Version {}\n", CNOID_FULL_VERSION_STRING) +
            "\n" +
            "Copyright (c) 2024 Japan Atomic Energy Agency.\n"
            "\n" +
            MITLicenseText();
        return text.c_str();
    }
};

}

CNOID_IMPLEMENT_PLUGIN_ENTRY(SimpleColliderPlugin)