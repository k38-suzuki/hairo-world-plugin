/**
   \file
   \author Kenta Suzuki
*/

#include <cnoid/Plugin>
#include <fmt/format.h>
// #include "LayoutSwitcherBar.h"
#include "LayoutSwitcherView.h"

using namespace cnoid;

namespace {

class LayoutSwitcherPlugin : public Plugin
{
public:

    LayoutSwitcherPlugin() : Plugin("LayoutSwitcher")
    {

    }

    virtual bool initialize()
    {
        // LayoutSwitcherBar::initialize(this);
        LayoutSwitcherView::initializeClass(this);
        return true;
    }

    virtual const char* description() const override
    {
        static std::string text =
            fmt::format("LayoutSwitcher Plugin Version {}\n", CNOID_FULL_VERSION_STRING) +
            "\n" +
            "Copyright (c) 2023 Japan Atomic Energy Agency.\n"
            "\n" +
            MITLicenseText();
        return text.c_str();
    }
};

}

CNOID_IMPLEMENT_PLUGIN_ENTRY(LayoutSwitcherPlugin)
