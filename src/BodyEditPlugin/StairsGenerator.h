/**
   @author Kenta Suzuki
*/

#ifndef CNOID_BODY_EDIT_PLUGIN_STAIRS_GENERATOR_H
#define CNOID_BODY_EDIT_PLUGIN_STAIRS_GENERATOR_H

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

#endif
