/**
   @author Kenta Suzuki
*/

#include <cnoid/Format>
#include <cnoid/Plugin>
#include "BentPipeGenerator.h"
#include "BodyConverter.h"
#include "CrawlerGenerator.h"
#include "GratingGenerator.h"
#include "InertiaCalculator.h"
#include "PipeGenerator.h"
#include "SlopeGenerator.h"
#include "StairsGenerator.h"
#include "TerrainGenerator.h"

using namespace cnoid;

class BodyEditPlugin : public Plugin
{
public:

    BodyEditPlugin() : Plugin("BodyEdit")
    {
        require("Body");
    }

    virtual bool initialize() override
    {
        BodyConverter::initializeClass(this);
        CrawlerGenerator::initializeClass(this);
        GratingGenerator::initializeClass(this);
        InertiaCalculator::initializeClass(this);
        PipeGenerator::initializeClass(this);
        BentPipeGenerator::initializeClass(this);
        SlopeGenerator::initializeClass(this);
        StairsGenerator::initializeClass(this);
        TerrainGenerator::initializeClass(this);
        return true;
    }

    virtual const char* description() const override
    {
        static std::string text =
            formatC("BodyEdit Plugin Version {}\n", CNOID_FULL_VERSION_STRING) +
            "\n" +
            "Copyright (c) 2023 Japan Atomic Energy Agency.\n"
            "\n" +
            MITLicenseText();
        return text.c_str();
    }
};

CNOID_IMPLEMENT_PLUGIN_ENTRY(BodyEditPlugin)