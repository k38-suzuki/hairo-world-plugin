/**
   \file
   \author Kenta Suzuki
*/

#ifndef CNOID_BODYGENERATORPLUGIN_PIPEGENERATOR_H
#define CNOID_BODYGENERATORPLUGIN_PIPEGENERATOR_H

#include <cnoid/ExtensionManager>

namespace cnoid {

class PipeGeneratorImpl;

class PipeGenerator
{
public:
    PipeGenerator();
    virtual ~PipeGenerator();

    static void initialize(ExtensionManager* ext);

    void show();

private:
    PipeGeneratorImpl* impl;
    friend class PipeGeneratorImpl;
};

}

#endif // CNOID_BODYGENERATORPLUGIN_PIPEGENERATOR_H
