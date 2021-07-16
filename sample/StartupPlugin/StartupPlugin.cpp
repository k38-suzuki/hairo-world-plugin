/**
   \file
   \author Kenta Suzuki
*/

#include <cnoid/MessageView>
#include <cnoid/Plugin>
#include <fmt/format.h>
#include <QSystemTrayIcon>
#include "StartupDialog.h"
#include "gettext.h"

using namespace cnoid;

namespace {

class StartupPlugin : public Plugin
{
public:

    StartupPlugin() : Plugin("Startup")
    {

    }

    virtual bool initialize() override
    {
        if(!QSystemTrayIcon::isSystemTrayAvailable()) {
            MessageView::instance()
                    ->putln(fmt::format("I couldn't detect any system tray on this system."));
        }

        StartupDialog::initializeClass(this);
        return true;
    }

    virtual const char* description() const override
    {
        static std::string text =
            fmt::format("Startup Plugin Version {}\n", CNOID_FULL_VERSION_STRING) +
            "\n" +
            "Copyright (c) 2021 Japan Atomic Energy Agency.\n"
            "\n" +
            MITLicenseText();
        return text.c_str();
    }
};

}

CNOID_IMPLEMENT_PLUGIN_ENTRY(StartupPlugin)
