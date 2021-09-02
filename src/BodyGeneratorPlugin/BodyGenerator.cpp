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

namespace cnoid {

class BodyGeneratorImpl
{
public:
    BodyGeneratorImpl(BodyGenerator* self);
    BodyGenerator* self;
};

}


BodyGenerator::BodyGenerator()
{
    impl = new BodyGeneratorImpl(this);
}


BodyGeneratorImpl::BodyGeneratorImpl(BodyGenerator* self)
    : self(self)
{

}


BodyGenerator::~BodyGenerator()
{
    delete impl;
}


void BodyGenerator::initializeClass(ExtensionManager* ext)
{
    MenuManager& menuManager = ext->menuManager();

    const char* builders[] = { _("Pipe"), _("Grating"), _("Slope"), _("BoxTerrain"), _("CrawlerRobot") };
    menuManager.setPath("/Tools").setPath(_("BodyGenerator"));
    menuManager.addItem(builders[0])
            ->sigTriggered().connect([&](){ PipeBuilderDialog::instance()->show(); });
    menuManager.addItem(builders[1])
            ->sigTriggered().connect([&](){ GratingBuilderDialog::instance()->show(); });
    menuManager.addItem(builders[2])
            ->sigTriggered().connect([&](){ SlopeBuilderDialog::instance()->show(); });
    menuManager.addItem(builders[3])
            ->sigTriggered().connect([&](){ TerrainBuilderDialog::instance()->show(); });
    menuManager.addItem(builders[4])
            ->sigTriggered().connect([&](){ CrawlerBuilderDialog::instance()->show(); });
}
