/**
   \file
   \author Kenta Suzuki
*/

#include <cnoid/Plugin>
#include <fmt/format.h>
#include "CollisionVisualizerItem.h"
using namespace cnoid;

namespace {

class CollisionSeqPlugin : public Plugin
{
public:

    CollisionSeqPlugin() : Plugin("CollisionSeq")
    {
        require("Body");
    }

    virtual bool initialize()
    {
        CollisionVisualizerItem::initializeClass(this);
        return true;
    }

    virtual const char* description() const override
    {
        static std::string text =
            fmt::format("CollisionSeq Plugin Version {}\n", CNOID_FULL_VERSION_STRING) +
            "\n" +
            "Copyright (c) 2021 Japan Atomic Energy Agency.\n"
            "\n" +
            MITLicenseText();
        return text.c_str();
    }
};

}

CNOID_IMPLEMENT_PLUGIN_ENTRY(CollisionSeqPlugin)
