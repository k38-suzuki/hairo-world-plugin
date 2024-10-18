/**
   @author Kenta Suzuki
*/

#include <cnoid/Format>
#include <cnoid/Plugin>
#include "IKPlannerItem.h"

using namespace cnoid;

class IKPlannerPlugin : public Plugin
{
public:

    IKPlannerPlugin() : Plugin("IKPlanner")
    {
        require("Body");
        require("OMPL");
    }

    virtual bool initialize() override
    {
        IKPlannerItem::initializeClass(this);
        return true;
    }

    virtual const char* description() const override
    {
        static std::string text =
            formatC("IKPlanner Plugin Version {}\n", CNOID_FULL_VERSION_STRING) +
            "\n" +
            "Copyrigh (c) 2022 Japan Atomic Energy Agency.\n"
            "\n" +
            MITLicenseText();
        return text.c_str();
    }
};

CNOID_IMPLEMENT_PLUGIN_ENTRY(IKPlannerPlugin)