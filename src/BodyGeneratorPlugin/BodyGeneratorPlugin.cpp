/**
   \file
   \author Kenta Suzuki
*/

#include <cnoid/MenuManager>
#include <cnoid/Plugin>
#include <fmt/format.h>
#include "PipeGeneratorDialog.h"
#include "GratingGeneratorDialog.h"
#include "SlopeGeneratorDialog.h"
#include "TerrainGeneratorDialog.h"
#include "CrawlerGeneratorDialog.h"
#include "gettext.h"

using namespace cnoid;

namespace {

class BodyGeneratorPlugin : public Plugin
{
public:

    BodyGeneratorPlugin() : Plugin("BodyGenerator")
    {
        require("Body");
    }

    virtual bool initialize()
    {
        MenuManager& mm = menuManager().setPath("/" N_("Tools")).setPath(_("BodyGenerator"));
        mm.addItem(_("CrawlerRobot"))->sigTriggered().connect(
                    [&](){ CrawlerGeneratorDialog::instance()->show(); });
        mm.addItem(_("Grating"))->sigTriggered().connect(
                    [&](){ GratingGeneratorDialog::instance()->show(); });
        mm.addItem(_("Pipe"))->sigTriggered().connect(
                    [&](){ PipeGeneratorDialog::instance()->show(); });
        mm.addItem(_("Slope"))->sigTriggered().connect(
                    [&](){ SlopeGeneratorDialog::instance()->show(); });
        mm.addItem(_("BoxTerrain"))->sigTriggered().connect(
                    [&](){ TerrainGeneratorDialog::instance()->show(); });
        return true;
    }

    virtual const char* description() const override
    {
        static std::string text =
            fmt::format("BodyGenerator Plugin Version {}\n", CNOID_FULL_VERSION_STRING) +
            "\n" +
            "Copyright (c) 2023 Japan Atomic Energy Agency.\n"
            "\n" +
            MITLicenseText();
        return text.c_str();
    }
};

}

CNOID_IMPLEMENT_PLUGIN_ENTRY(BodyGeneratorPlugin)