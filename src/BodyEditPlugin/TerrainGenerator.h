/**
   @author Kenta Suzuki
*/

#ifndef CNOID_BODYEDIT_PLUGIN_TERRAIN_GENERATOR_H
#define CNOID_BODYEDIT_PLUGIN_TERRAIN_GENERATOR_H

namespace cnoid {

class ExtensionManager;

class TerrainGenerator
{
public:
    TerrainGenerator();
    virtual ~TerrainGenerator();

    static void initializeClass(ExtensionManager* ext);

private:
    class Impl;
    Impl* impl;
};

}

#endif // CNOID_BODYEDIT_PLUGIN_TERRAIN_GENERATOR_H
