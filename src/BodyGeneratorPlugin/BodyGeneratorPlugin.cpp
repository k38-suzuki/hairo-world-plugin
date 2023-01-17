/**
   \file
   \author Kenta Suzuki
*/

#include <cnoid/Plugin>
#include <fmt/format.h>
#include "CrawlerGenerator.h"
#include "GratingGenerator.h"
#include "PipeGenerator.h"
#include "SlopeGenerator.h"
#include "TerrainGenerator.h"

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
        CrawlerGenerator::initializeClass(this);
        GratingGenerator::initializeClass(this);
        PipeGenerator::initializeClass(this);
        SlopeGenerator::initializeClass(this);
        TerrainGenerator::initializeClass(this);
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