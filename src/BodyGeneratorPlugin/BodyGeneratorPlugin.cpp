/**
   \file
   \author Kenta Suzuki
*/

#include <cnoid/Plugin>
#include <fmt/format.h>
#include "PipeGenerator.h"
#include "GratingGenerator.h"
#include "SlopeGenerator.h"
#include "TerrainGenerator.h"
#include "CrawlerGenerator.h"

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
        PipeGenerator::initializeClass(this);
        GratingGenerator::initializeClass(this);
        SlopeGenerator::initializeClass(this);
        TerrainGenerator::initializeClass(this);
        CrawlerGenerator::initializeClass(this);
        return true;
    }

    virtual const char* description() const override
    {
        static std::string text =
            fmt::format("BodyGenerator Plugin Version {}\n", CNOID_FULL_VERSION_STRING) +
            "\n" +
            "Copyright (c) 2021 Japan Atomic Energy Agency.\n"
            "\n" +
            MITLicenseText();
        return text.c_str();
    }
};

}

CNOID_IMPLEMENT_PLUGIN_ENTRY(BodyGeneratorPlugin)