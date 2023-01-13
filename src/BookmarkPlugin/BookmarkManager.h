/**
   \file
   \author Kenta Suzuki
*/

#ifndef CNOID_KIOSK_PLUGIN_BOOKMARK_MANAGER_H
#define CNOID_KIOSK_PLUGIN_BOOKMARK_MANAGER_H

#include <cnoid/ExtensionManager>

namespace cnoid {

class BookmarkManagerImpl;

class BookmarkManager
{
public:
    BookmarkManager();
    virtual ~BookmarkManager();

    static void initializeClass(ExtensionManager* ext);
    static BookmarkManager* instance();

    void showBookmarkManagerDialog();

private:
    BookmarkManagerImpl* impl;
    friend class BookmarkManagerImpl;
};

}

#endif // CNOID_KIOSK_PLUGIN_BOOKMARK_MANAGER_H