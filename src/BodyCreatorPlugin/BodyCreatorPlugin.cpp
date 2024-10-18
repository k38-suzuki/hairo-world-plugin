/**
   @author Kenta Suzuki
*/

#include <cnoid/Format>
#include <cnoid/Plugin>
#include "BodyCreator.h"
#include "BodyCreatorDialog.h"

using namespace cnoid;

class BodyCreatorPlugin : public Plugin
{
public:

    BodyCreatorPlugin() : Plugin("BodyCreator")
    {
        require("Body");
        require("Bookmark");
    }

    virtual bool initialize() override
    {
        BodyCreatorDialog::initializeClass(this);
        BentPipeCreator::initializeClass(this);
        CrawlerCreator::initializeClass(this);
        GratingCreator::initializeClass(this);
        PipeCreator::initializeClass(this);
        SlopeCreator::initializeClass(this);
        StairsCreator::initializeClass(this);
        TerrainCreator::initializeClass(this);
        FormatConverter::initializeClass(this);
        InertiaCalculator::initializeClass(this);
        return true;
    }

    virtual const char* description() const override
    {
        static std::string text =
            formatC("BodyCreator Plugin Version {}\n", CNOID_FULL_VERSION_STRING) +
            "\n" +
            "Copyright (c) 2023 Japan Atomic Energy Agency.\n"
            "\n" +
            MITLicenseText();
        return text.c_str();
    }
};

CNOID_IMPLEMENT_PLUGIN_ENTRY(BodyCreatorPlugin)