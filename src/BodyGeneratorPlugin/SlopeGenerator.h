/**
   \file
   \author Kenta Suzuki
*/

#ifndef CNOID_BODYGENERATORPLUGIN_SLOPEGENERATOR_H
#define CNOID_BODYGENERATORPLUGIN_SLOPEGENERATOR_H

#include <cnoid/ExtensionManager>

namespace cnoid {

class SlopeGeneratorImpl;

class SlopeGenerator
{
public:
    SlopeGenerator();
    virtual ~SlopeGenerator();

    static void initializeClass(ExtensionManager* ext);

private:
    SlopeGeneratorImpl* impl;
    friend class SlopeGeneratorImpl;
};

}

#endif // CNOID_BODYGENERATORPLUGIN_SLOPEGENERATOR_H
