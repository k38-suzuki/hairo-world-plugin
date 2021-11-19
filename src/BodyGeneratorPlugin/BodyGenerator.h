/**
   \file
   \author Kenta Suzuki
*/

#ifndef CNOID_BODYGENERATORPLUGIN_BODYGENERATOR_H
#define CNOID_BODYGENERATORPLUGIN_BODYGENERATOR_H

#include <cnoid/ExtensionManager>

namespace cnoid {

class BodyGenerator
{
public:
    BodyGenerator();
    virtual ~BodyGenerator();

    static void initialize(ExtensionManager* ext);
};

}

#endif // CNOID_BODYGENERATORPLUGIN_BODYGENERATOR_H
