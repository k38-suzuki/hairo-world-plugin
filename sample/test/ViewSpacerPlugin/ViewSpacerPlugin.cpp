/**
   \file
   \author Kenta Suzuki
*/

#include <cnoid/Plugin>
#include <fmt/format.h>
#include "EmptyView.h"

using namespace cnoid;

class ViewSpacerPlugin : public Plugin
{
public:

    ViewSpacerPlugin() : Plugin("ViewSpacer")
    {

    }

    virtual bool initialize() override
    {
        EmptyView::initializeClass(this);
        return true;
    }

    virtual const char* description() const override
    {
        static std::string text =
            fmt::format("ViewSpacer Plugin Version {}\n", CNOID_FULL_VERSION_STRING) +
            "\n" +
            "Copyright (c) 2021 Japan Atomic Energy Agency.\n"
            "\n" +
            MITLicenseText();
        return text.c_str();
    }
};

CNOID_IMPLEMENT_PLUGIN_ENTRY(ViewSpacerPlugin)
