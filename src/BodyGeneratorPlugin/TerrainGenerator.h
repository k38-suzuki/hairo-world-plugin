/**
   \file
   \author Kenta Suzuki
*/

#ifndef CNOID_BODY_GENERATOR_PLUGIN_TERRAIN_GENERATOR_H
#define CNOID_BODY_GENERATOR_PLUGIN_TERRAIN_GENERATOR_H

#include <cnoid/ExtensionManager>

namespace cnoid {

class TerrainGeneratorImpl;

class TerrainGenerator
{
public:
    TerrainGenerator();
    virtual ~TerrainGenerator();

    static void initializeClass(ExtensionManager* ext);

private:
    TerrainGeneratorImpl* impl;
    friend class TerrainGeneratorImpl;
};

}

#endif // CNOID_BODY_GENERATOR_PLUGIN_TERRAIN_GENERATOR_H