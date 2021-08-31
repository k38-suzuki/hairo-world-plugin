/**
   \file
   \author Kenta Suzuki
*/

#ifndef CNOID_BOOKMARKPLUGIN_BOOKMARKBAR_H
#define CNOID_BOOKMARKPLUGIN_BOOKMARKBAR_H

#include <cnoid/ExtensionManager>
#include <cnoid/ToolBar>

namespace cnoid {

class BookmarkBarImpl;

class BookmarkBar : public ToolBar
{
public:
    BookmarkBar();
    virtual ~BookmarkBar();

    static void initializeClass(ExtensionManager* ext);
    static BookmarkBar* instance();

private:
    BookmarkBarImpl* impl;
    friend class BookmarkBarImpl;
};

}

#endif // CNOID_BOOKMARKPLUGIN_BOOKMARKBAR_H
