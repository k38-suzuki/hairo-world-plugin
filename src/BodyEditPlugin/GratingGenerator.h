/**
   @author Kenta Suzuki
*/

#ifndef CNOID_BODY_EDIT_PLUGIN_GRATING_GENERATOR_H
#define CNOID_BODY_EDIT_PLUGIN_GRATING_GENERATOR_H

#include <cnoid/ExtensionManager>

namespace cnoid {

class GratingGenerator
{
public:
    GratingGenerator();
    virtual ~GratingGenerator();

    static void initializeClass(ExtensionManager* ext);

private:
    class Impl;
    Impl* impl;
};

}

#endif
