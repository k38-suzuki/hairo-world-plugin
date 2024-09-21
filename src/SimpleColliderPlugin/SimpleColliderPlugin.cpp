/**
   @author Kenta Suzuki
*/

#include <cnoid/Format>
#include <cnoid/Plugin>
#include "SimpleColliderItem.h"
#include "MultiColliderItem.h"

using namespace cnoid;

class SimpleColliderPlugin : public Plugin
{
public:
    SimpleColliderPlugin() : Plugin("SimpleCollider")
    {
        require("Body");
    }

    virtual bool initialize() override
    {
        SimpleColliderItem::initializeClass(this);
        MultiColliderItem::initializeClass(this);
        return true;
    }

    virtual const char* description() const override
    {
        static std::string text =
            formatC("SimpleCollider Plugin Version {}\n", CNOID_FULL_VERSION_STRING) +
            "\n" +
            "Copyright (c) 2024 Japan Atomic Energy Agency.\n"
            "\n" +
            MITLicenseText();
        return text.c_str();
    }
};

CNOID_IMPLEMENT_PLUGIN_ENTRY(SimpleColliderPlugin)