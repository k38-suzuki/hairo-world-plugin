/**
   @author Kenta Suzuki
*/

#ifndef CNOID_BODY_EDIT_PLUGIN_SLOPE_GENERATOR_H
#define CNOID_BODY_EDIT_PLUGIN_SLOPE_GENERATOR_H

#include <cnoid/ExtensionManager>

namespace cnoid {

class SlopeGeneratorImpl;

class SlopeGenerator
{
public:
    SlopeGenerator();
    virtual ~SlopeGenerator();

    static void initializeClass(ExtensionManager* ext);

private:
    SlopeGeneratorImpl* impl;
    friend class SlopeGeneratorImpl;
};

}

#endif
