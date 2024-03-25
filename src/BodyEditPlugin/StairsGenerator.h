/**
   @author Kenta Suzuki
*/

#ifndef CNOID_BODYEDIT_PLUGIN_STAIRS_GENERATOR_H
#define CNOID_BODYEDIT_PLUGIN_STAIRS_GENERATOR_H

namespace cnoid {

class ExtensionManager;

class StairsGenerator
{
public:
    StairsGenerator();
    virtual ~StairsGenerator();

    static void initializeClass(ExtensionManager* ext);

private:
    class Impl;
    Impl* impl;
};

}

#endif // CNOID_BODYEDIT_PLUGIN_STAIRS_GENERATOR_H
