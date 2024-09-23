/**
   @author Kenta Suzuki
*/

#ifndef CNOID_BODYEDIT_PLUGIN_SLOPE_GENERATOR_H
#define CNOID_BODYEDIT_PLUGIN_SLOPE_GENERATOR_H

namespace cnoid {

class ExtensionManager;

class SlopeGenerator
{
public:
    static void initializeClass(ExtensionManager* ext);

    SlopeGenerator();
    virtual ~SlopeGenerator();

private:
    class Impl;
    Impl* impl;
};

}

#endif // CNOID_BODYEDIT_PLUGIN_SLOPE_GENERATOR_H
