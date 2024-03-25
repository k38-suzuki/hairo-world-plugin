/**
   @author Kenta Suzuki
*/

#ifndef CNOID_BODYEDIT_PLUGIN_BODY_CONVERTER_H
#define CNOID_BODYEDIT_PLUGIN_BODY_CONVERTER_H

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

#endif // CNOID_BODYEDIT_PLUGIN_BODY_CONVERTER_H
