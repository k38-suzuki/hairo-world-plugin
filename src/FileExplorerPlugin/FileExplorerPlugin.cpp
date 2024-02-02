/**
   @author Kenta Suzuki
*/

#include <cnoid/Plugin>
#include <fmt/format.h>
#include "FileExplorer.h"

using namespace cnoid;

namespace {

class FileExplorerPlugin : public Plugin
{
public:

    FileExplorerPlugin() : Plugin("FileExplorer")
    {
        require("Body");
    }

    virtual bool initialize() override
    {
        FileExplorer::initializeClass(this);
        return true;
    }

    virtual const char* description() const override
    {
        static std::string text =
            fmt::format("FileExplorer Plugin Version {}\n", CNOID_FULL_VERSION_STRING) +
            "\n" +
            "Copyright (c) 2020 Japan Atomic Energy Agency.\n"
            "\n" +
            MITLicenseText();
        return text.c_str();
    }
};

}

CNOID_IMPLEMENT_PLUGIN_ENTRY(FileExplorerPlugin)