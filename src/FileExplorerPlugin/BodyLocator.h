/**
    @author Kenta Suzuki
*/

#ifndef CNOID_FILE_EXPLORER_PLUGIN_BODY_LOCATOR_H
#define CNOID_FILE_EXPLORER_PLUGIN_BODY_LOCATOR_H

namespace cnoid {

class ExtensionManager;

class BodyLocator
{
public:
    static void initializeClass(ExtensionManager* ext);

    BodyLocator();
    virtual ~BodyLocator();
};

}

#endif // CNOID_FILE_EXPLORE_PLUGIN_BODY_LOCATOR_H