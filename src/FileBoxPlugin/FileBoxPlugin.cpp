/**
   \file
   \author Kenta Suzuki
*/

#include <cnoid/Plugin>
#include <fmt/format.h>
#include "FileBoxView.h"

using namespace cnoid;

namespace {

class FileBoxPlugin : public Plugin
{
public:

    FileBoxPlugin() : Plugin("FileBox")
    {
        require("Body");
    }

    virtual bool initialize()
    {
        FileBoxView::initializeClass(this);
        return true;
    }

    virtual const char* description() const override
    {
        static std::string text =
            fmt::format("FileBox Plugin Version {}\n", CNOID_FULL_VERSION_STRING) +
            "\n" +
            "Copyright (c) 2020 Japan Atomic Energy Agency.\n"
            "\n" +
            MITLicenseText();
        return text.c_str();
    }
};

}

CNOID_IMPLEMENT_PLUGIN_ENTRY(FileBoxPlugin)
