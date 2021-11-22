/**
   \file
   \author Kenta Suzuki
*/

#ifndef CNOID_BODYGENERATORPLUGIN_TERRAINGENERATOR_H
#define CNOID_BODYGENERATORPLUGIN_TERRAINGENERATOR_H

#include <cnoid/ExtensionManager>

namespace cnoid {

class TerrainGeneratorImpl;

class TerrainGenerator
{
public:
    TerrainGenerator(ExtensionManager* ext);
    virtual ~TerrainGenerator();

    static void initialize(ExtensionManager* ext);

private:
    TerrainGeneratorImpl* impl;
    friend class TerrainGeneratorImpl;
};

}

#endif // CNOID_BODYGENERATORPLUGIN_TERRAINGENERATOR_H
