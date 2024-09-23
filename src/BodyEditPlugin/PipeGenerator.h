/**
   @author Kenta Suzuki
*/

#ifndef CNOID_BODYEDIT_PLUGIN_PIPE_GENERATOR_H
#define CNOID_BODYEDIT_PLUGIN_PIPE_GENERATOR_H

namespace cnoid {

class ExtensionManager;

class PipeGenerator
{
public:
    static void initializeClass(ExtensionManager* ext);

    PipeGenerator();
    virtual ~PipeGenerator();

private:
    class Impl;
    Impl* impl;
};

}

#endif // CNOID_BODYEDIT_PLUGIN_PIPE_GENERATOR_H
