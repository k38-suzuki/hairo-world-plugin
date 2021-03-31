/**
   \file
   \author Kenta Suzuki
*/

#include <cnoid/Plugin>
#include <fmt/format.h>
#include "ObjectBuilderDialog.h"

using namespace cnoid;

namespace {

class ObjectBuilderPlugin : public Plugin
{
public:

    ObjectBuilderPlugin() : Plugin("ObjectBuilder")
    {
        require("Body");
    }

    virtual bool initialize()
    {
        ObjectBuilderDialog::initializeClass(this);
        return true;
    }

    virtual const char* description() const override
    {
        static std::string text =
            fmt::format("ObjectBuilder Plugin Version {}\n", CNOID_FULL_VERSION_STRING) +
            "\n" +
            "Copyright (c) 2021 Japan Atomic Energy Agency.\n"
            "\n" +
            MITLicenseText();
        return text.c_str();
    }
};

}

CNOID_IMPLEMENT_PLUGIN_ENTRY(ObjectBuilderPlugin)
