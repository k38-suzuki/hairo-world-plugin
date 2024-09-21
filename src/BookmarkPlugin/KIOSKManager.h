/**
   @author Kenta Suzuki
*/

#ifndef CNOID_BOOKMARK_PLUGIN_KIOSK_MANAGER_H
#define CNOID_BOOKMARK_PLUGIN_KIOSK_MANAGER_H

namespace cnoid {

class ExtensionManager;

class KIOSKManager
{
public:
    static void initializeClass(ExtensionManager* ext);
    static KIOSKManager* instance();

    KIOSKManager();
    virtual ~KIOSKManager();

    void setKIOSKEnabled(bool checked);

private:
    class Impl;
    Impl* impl;
};

}

#endif // CNOID_BOOKMARK_PLUGIN_KIOSK_MANAGER_H