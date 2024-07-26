/**
   @author Kenta Suzuki
*/

#include <cnoid/Format>
#include <cnoid/Plugin>
#include "TaskCreator.h"

using namespace cnoid;

class TaskCreatorPlugin : public Plugin
{
public:
    TaskCreatorPlugin() : Plugin("TaskCreator")
    {
        require("Body");
    }
    
    virtual bool initialize() override
    {
        TaskCreator::initializeClass(this);
        return true;
    }

    virtual const char* description() const override
    {
        static std::string text =
            formatC("TaskCreator Plugin Version {}\n", CNOID_FULL_VERSION_STRING) +
            "\n" +
            "Copyright (c) 2024 Japan Atomic Energy Agency.\n"
            "\n" +
            MITLicenseText();
        return text.c_str();
    }
};

CNOID_IMPLEMENT_PLUGIN_ENTRY(TaskCreatorPlugin)