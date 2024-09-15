/**
    @author Kenta Suzuki
 */

#ifndef CNOID_FILE_EXPLORE_PLUGIN_BODY_LOCATOR_H
#define CNOID_FILE_EXPLORE_PLUGIN_BODY_LOCATOR_H

namespace cnoid {

class ExtensionManager;

class BodyLocator
{
public:
    static void initializeClass(ExtensionManager* ext);
    static BodyLocator* instance();

    BodyLocator();
    virtual ~BodyLocator();

private:
    class Impl;
    Impl* impl;
};

}

#endif // CNOID_FILE_EXPLORE_PLUGIN_BODY_LOCATOR_H