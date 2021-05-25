/*!
  @author Kenta Suzuki
*/

#include <cnoid/Plugin>
#include <fmt/format.h>
#include "ViewSwitcherDialog.h"

using namespace cnoid;

namespace {

class ViewSwitcherPlugin : public Plugin
{


public:
    ViewSwitcherPlugin() : Plugin("ViewSwitcher")
    {

    }

    virtual bool initialize() override
    {
        ViewSwitcherDialog::initializeClass(this);
        return true;
    }

    virtual bool finalize() override
    {
        ViewSwitcherDialog::finalizeClass();
        return true;
    }

    virtual const char* description() const override
    {
        static std::string text =
            fmt::format("ViewSwitcher Plugin Version {}\n", CNOID_FULL_VERSION_STRING) +
            "\n" +
            "Copyright (c) 2021 Japan Atomic Energy Agency.\n"
            "\n" +
            MITLicenseText();
        return text.c_str();
    }
};

}

CNOID_IMPLEMENT_PLUGIN_ENTRY(ViewSwitcherPlugin)
