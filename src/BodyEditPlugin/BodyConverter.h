/**
   @author Kenta Suzuki
*/

#ifndef CNOID_BODY_EDIT_PLUGIN_BODY_CONVERTER_H
#define CNOID_BODY_EDIT_PLUGIN_BODY_CONVERTER_H

namespace cnoid {

class ExtensionManager;

class BodyConverter
{
public:
    static void initializeClass(ExtensionManager* ext);
    static BodyConverter* instance();

    BodyConverter();
    virtual ~BodyConverter();

private:
    class Impl;
    Impl* impl;
};

}

#endif
