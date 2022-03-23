/**
   \file
   \author Kenta Suzuki
*/

#ifndef CNOID_BODYGENERATORPLUGIN_GRATINGGENERATOR_H
#define CNOID_BODYGENERATORPLUGIN_GRATINGGENERATOR_H

#include <cnoid/ExtensionManager>

namespace cnoid {

class GratingGeneratorImpl;

class GratingGenerator
{
public:
    GratingGenerator();
    virtual ~GratingGenerator();

    static void initializeClass(ExtensionManager* ext);

private:
    GratingGeneratorImpl* impl;
    friend class GratingGeneratorImpl;
};

}

#endif // CNOID_BODYGENERATORPLUGIN_GRATINGGENERATOR_H
