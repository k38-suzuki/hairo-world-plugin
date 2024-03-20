/**
   @author Kenta Suzuki
*/

#ifndef CNOID_BODY_EDIT_PLUGIN_SLOPE_GENERATOR_H
#define CNOID_BODY_EDIT_PLUGIN_SLOPE_GENERATOR_H

namespace cnoid {

class ExtensionManager;

class SlopeGenerator
{
public:
    SlopeGenerator();
    virtual ~SlopeGenerator();

    static void initializeClass(ExtensionManager* ext);

private:
    class Impl;
    Impl* impl;
};

}

#endif
