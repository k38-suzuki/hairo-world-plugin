/**
   @author Kenta Suzuki
*/

#ifndef CNOID_BODY_EDIT_PLUGIN_PIPE_GENERATOR_H
#define CNOID_BODY_EDIT_PLUGIN_PIPE_GENERATOR_H

namespace cnoid {

class ExtensionManager;

class PipeGenerator
{
public:
    PipeGenerator();
    virtual ~PipeGenerator();

    static void initializeClass(ExtensionManager* ext);

private:
    class Impl;
    Impl* impl;
};

}

#endif
