/**
   @author Kenta Suzuki
*/

#ifndef CNOID_BODY_EDIT_PLUGIN_TERRAIN_GENERATOR_H
#define CNOID_BODY_EDIT_PLUGIN_TERRAIN_GENERATOR_H

#include <cnoid/ExtensionManager>

namespace cnoid {

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

#endif
