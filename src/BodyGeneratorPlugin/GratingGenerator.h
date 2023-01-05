/**
   \file
   \author Kenta Suzuki
*/

#ifndef CNOID_BODY_GENERATOR_PLUGIN_GRATING_GENERATOR_H
#define CNOID_BODY_GENERATOR_PLUGIN_GRATING_GENERATOR_H

#include <cnoid/ExtensionManager>

namespace cnoid {

class GratingGeneratorImpl;

class GratingGenerator
{
public:
    GratingGenerator();
    virtual ~GratingGenerator();

    static void initializeClass(ExtensionManager* ext);

private:
    GratingGeneratorImpl* impl;
    friend class GratingGeneratorImpl;
};

}

#endif // CNOID_BODY_GENERATOR_PLUGIN_GRATING_GENERATOR_H