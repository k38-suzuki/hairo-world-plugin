/**
   \file
   \author Kenta Suzuki
*/

#ifndef CNOID_BODY_GENERATOR_PLUGIN_STAIRS_GENERATOR_H
#define CNOID_BODY_GENERATOR_PLUGIN_STAIRS_GENERATOR_H

#include <cnoid/ExtensionManager>

namespace cnoid {

class StairsGeneratorImpl;

class StairsGenerator
{
public:
    StairsGenerator();
    virtual ~StairsGenerator();

    static void initializeClass(ExtensionManager* ext);

private:
    StairsGeneratorImpl* impl;
    friend class StairsGeneratorImpl;
};

}

#endif // CNOID_BODY_GENERATOR_PLUGIN_STAIRS_GENERATOR_H
