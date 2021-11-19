/**
   \file
   \author Kenta Suzuki
*/

#include "BodyGenerator.h"
#include <cnoid/MenuManager>
#include "CrawlerBuilderDialog.h"
#include "GratingBuilderDialog.h"
#include "PipeBuilderDialog.h"
#include "SlopeBuilderDialog.h"
#include "TerrainBuilderDialog.h"
#include "gettext.h"

using namespace cnoid;

BodyGenerator::BodyGenerator()
{

}


BodyGenerator::~BodyGenerator()
{

}


void BodyGenerator::initialize(ExtensionManager* ext)
{
    PipeBuilderDialog* pipeBuilder = ext->manage(new PipeBuilderDialog);
    GratingBuilderDialog* gratingBuilder = ext->manage(new GratingBuilderDialog);
    SlopeBuilderDialog* slopeBuilder = ext->manage(new SlopeBuilderDialog);
    TerrainBuilderDialog* terrainBuilder = ext->manage(TerrainBuilderDialog::instance());
    CrawlerBuilderDialog* crawlerBuilder = ext->manage(new CrawlerBuilderDialog);

    const char* labels[] = { _("Pipe"), _("Grating"), _("Slope"), _("BoxTerrain"), _("CrawlerRobot") };
    MenuManager& mm = ext->menuManager();
    mm.setPath("/Tools").setPath(_("BodyGenerator"));
    mm.addItem(labels[0])->sigTriggered().connect([=](){ pipeBuilder->show(); });
    mm.addItem(labels[1])->sigTriggered().connect([=](){ gratingBuilder->show(); });
    mm.addItem(labels[2])->sigTriggered().connect([=](){ slopeBuilder->show(); });
    mm.addItem(labels[3])->sigTriggered().connect([=](){ terrainBuilder->show(); });
    mm.addItem(labels[4])->sigTriggered().connect([=](){ crawlerBuilder->show(); });
}
