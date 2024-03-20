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
    static void initializeClass(ExtensionManager* ext);
    static BodyConverter* instance();

    BodyConverter();
    ~BodyConverter();

private:
    class Impl;
    Impl* impl;
};

}

#endif
