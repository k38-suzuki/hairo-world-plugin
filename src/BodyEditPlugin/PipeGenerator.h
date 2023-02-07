/**
   \file
   \author Kenta Suzuki
*/

#ifndef CNOID_BODY_GENERATOR_PLUGIN_PIPE_GENERATOR_H
#define CNOID_BODY_GENERATOR_PLUGIN_PIPE_GENERATOR_H

#include <cnoid/ExtensionManager>

namespace cnoid {

class PipeGeneratorImpl;

class PipeGenerator
{
public:
    PipeGenerator();
    virtual ~PipeGenerator();

    static void initializeClass(ExtensionManager* ext);

private:
    PipeGeneratorImpl* impl;
    friend class PipeGeneratorImpl;
};

}

#endif // CNOID_BODY_GENERATOR_PLUGIN_PIPE_GENERATOR_H