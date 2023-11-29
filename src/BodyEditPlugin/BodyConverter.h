/**
   \file
   \author Kenta Suzuki
*/

#ifndef CNOID_BODY_EDIT_PLUGIN_BODY_CONVERTER_H
#define CNOID_BODY_EDIT_PLUGIN_BODY_CONVERTER_H

#include <cnoid/ExtensionManager>

namespace cnoid {

class BodyConverterImpl;

class BodyConverter
{
public:
    BodyConverter();
    virtual ~BodyConverter();

    static void initializeClass(ExtensionManager* ext);

private:
    BodyConverterImpl* impl;
    friend class BodyConverterImpl;
};

}

#endif // CNOID_BODY_EDIT_PLUGIN_BODY_CONVERTER_H