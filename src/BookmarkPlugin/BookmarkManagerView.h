/**
   \file
   \author Kenta Suzuki
*/

#ifndef CNOID_BOOKMARKPLUGIN_BOOKMARKMANAGERVIEW_H
#define CNOID_BOOKMARKPLUGIN_BOOKMARKMANAGERVIEW_H

#include <cnoid/ExtensionManager>
#include <cnoid/View>

namespace cnoid {

class BookmarkManagerViewImpl;

class BookmarkManagerView : public View
{
public:
    BookmarkManagerView();
    virtual ~BookmarkManagerView();

    static void initializeClass(ExtensionManager* ext);
    static bool openDialogToLoadProject(const std::string& filename);

    virtual bool storeState(Archive& archive);
    virtual bool restoreState(const Archive& archive);

protected:
    virtual void onActivated();
    virtual void onDeactivated();

private:
    BookmarkManagerViewImpl* impl;
    friend class BookmarkManagerViewImpl;
};

}

#endif // CNOID_BOOKMARKPLUGIN_BOOKMARKMANAGERVIEW_H
