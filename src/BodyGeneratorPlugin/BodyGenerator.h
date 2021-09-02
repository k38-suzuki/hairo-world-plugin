/**
   \file
   \author Kenta Suzuki
*/

#ifndef CNOID_BODYGENERATORPLUGIN_BODYGENERATOR_H
#define CNOID_BODYGENERATORPLUGIN_BODYGENERATOR_H

#include <cnoid/ExtensionManager>

namespace cnoid {

class BodyGeneratorImpl;

class BodyGenerator
{
public:
    BodyGenerator();
    virtual ~BodyGenerator();

    static void initializeClass(ExtensionManager* ext);

private:
    BodyGeneratorImpl* impl;
    friend class BodyGeneratorImpl;
};

}

#endif // CNOID_BODYGENERATORPLUGIN_BODYGENERATOR_H
