/**
   \file
   \author Kenta Suzuki
*/

#include <cnoid/Plugin>
#include <fmt/format.h>
#include "CollisionVisualizerItem.h"
#include "OperationReviewCollectorItem.h"
using namespace cnoid;

namespace {

class OperationReviewPlugin : public Plugin
{
public:

    OperationReviewPlugin() : Plugin("OperationReview")
    {
        require("Body");
    }

    virtual bool initialize()
    {
        CollisionVisualizerItem::initializeClass(this);
        OperationReviewCollectorItem::initializeClass(this);
        return true;
    }

    virtual const char* description() const override
    {
        static std::string text =
            fmt::format("OperationReview Plugin Version {}\n", CNOID_FULL_VERSION_STRING) +
            "\n" +
            "Copyright (c) 2021 Japan Atomic Energy Agency.\n"
            "\n" +
            MITLicenseText();
        return text.c_str();
    }
};

}

CNOID_IMPLEMENT_PLUGIN_ENTRY(OperationReviewPlugin)
