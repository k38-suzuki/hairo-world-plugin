/**
   @author Kenta Suzuki
*/

#ifndef CNOID_BODY_EDIT_PLUGIN_BODY_CONVERTER_H
#define CNOID_BODY_EDIT_PLUGIN_BODY_CONVERTER_H

#include <cnoid/ExtensionManager>

namespace cnoid {

class BodyConverter
{
public:
    BodyConverter();
    virtual ~BodyConverter();

    static void initializeClass(ExtensionManager* ext);

private:
    class Impl;
    Impl* impl;
};

}

#endif
