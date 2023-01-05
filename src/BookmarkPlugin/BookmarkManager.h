/**
   \file
   \author Kenta Suzuki
*/

#ifndef CNOID_BOOKMARK_PLUGIN_BOOKMARK_MANAGER_H
#define CNOID_BOOKMARK_PLUGIN_BOOKMARK_MANAGER_H

#include <cnoid/ExtensionManager>

namespace cnoid {

class BookmarkManagerImpl;

class BookmarkManager
{
public:
    BookmarkManager(ExtensionManager* ext);
    virtual ~BookmarkManager();

    static void initializeClass(ExtensionManager* ext);

private:
    BookmarkManagerImpl* impl;
    friend class BookmarkManagerImpl;
};

}

#endif // CNOID_BOOKMARK_PLUGIN_BOOKMARK_MANAGER_H