/**
   @author Kenta Suzuki
*/

#include <cnoid/Plugin>
#include "LiftSimulatorItem.h"

using namespace cnoid;

class LiftSamplePlugin : public Plugin
{
public:

    LiftSamplePlugin() : Plugin("LiftSample")
    {
        require("Body");
        require("CFD");
    }

    virtual bool initialize() override
    {
        LiftSimulatorItem::initializeClass(this);
        return true;
    }
};

CNOID_IMPLEMENT_PLUGIN_ENTRY(LiftSamplePlugin)