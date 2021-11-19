/**
   \file
   \author Kenta Suzuki
*/

#ifndef CNOID_BOOKMARKPLUGIN_BOOKMARKMANAGER_H
#define CNOID_BOOKMARKPLUGIN_BOOKMARKMANAGER_H

#include <cnoid/ExtensionManager>

namespace cnoid {

class BookmarkManagerImpl;

class BookmarkManager
{
public:
    BookmarkManager();
    virtual ~BookmarkManager();

    static void initialize(ExtensionManager* ext);

    void show();

private:
    BookmarkManagerImpl* impl;
    friend class BookmarkManagerImpl;
};

}

#endif // CNOID_BOOKMARKPLUGIN_BOOKMARKMANAGER_H
