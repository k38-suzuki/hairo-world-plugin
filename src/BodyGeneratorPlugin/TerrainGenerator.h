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
    TerrainGenerator();
    virtual ~TerrainGenerator();

    static void initialize(ExtensionManager* ext);
    static TerrainGenerator* instance();

    void show();
    double scale() const;

private:
    TerrainGeneratorImpl* impl;
    friend class TerrainGeneratorImpl;
};

}

#endif // CNOID_BODYGENERATORPLUGIN_TERRAINGENERATOR_H
