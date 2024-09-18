/**
   @author Kenta Suzuki
*/

#include <cnoid/Format>
#include <cnoid/Plugin>
#include "CollisionLoggerItem.h"

using namespace cnoid;

class CollisionSeqPlugin : public Plugin
{
public:
    CollisionSeqPlugin() : Plugin("CollisionSeq")
    {
        require("Body");
    }

    virtual bool initialize() override
    {
        CollisionLoggerItem::initializeClass(this);
        return true;
    }

    virtual const char* description() const override
    {
        static std::string text =
            formatC("CollisionSeq Plugin Version {}\n", CNOID_FULL_VERSION_STRING) +
            "\n" +
            "Copyright (c) 2021 Japan Atomic Energy Agency.\n"
            "\n" +
            MITLicenseText();
        return text.c_str();
    }
};

CNOID_IMPLEMENT_PLUGIN_ENTRY(CollisionSeqPlugin)