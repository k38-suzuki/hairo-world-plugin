/**
   \file
   \author Kenta Suzuki
*/

#ifndef CNOID_BOOKMARKPLUGIN_BOOKMARKLIST_H
#define CNOID_BOOKMARKPLUGIN_BOOKMARKLIST_H

#include <cnoid/ExtensionManager>

namespace cnoid {

class BookmarkListImpl;

class BookmarkList
{
public:
    BookmarkList(ExtensionManager* ext);
    virtual ~BookmarkList();

    static void initialize(ExtensionManager* ext);

private:
    BookmarkListImpl* impl;
    friend class BookmarkListImpl;
};

}

#endif // CNOID_BOOKMARKPLUGIN_BOOKMARKLIST_H
