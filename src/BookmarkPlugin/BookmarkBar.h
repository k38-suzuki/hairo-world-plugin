/**
   \file
   \author Kenta Suzuki
*/

#ifndef MARK_PLUGIN_BOOKMARK_BAR_H
#define MARK_PLUGIN_BOOKMARK_BAR_H

#include <cnoid/ExtensionManager>
#include <cnoid/ToolBar>

namespace cnoid {

class BookmarkBarImpl;

class BookmarkBar : public ToolBar
{
public:
    BookmarkBar(ExtensionManager* ext);
    virtual ~BookmarkBar();

    static void initializeClass(ExtensionManager* ext);
    static BookmarkBar* instance();

private:
    BookmarkBarImpl* impl;
    friend class BookmarkBarImpl;
};

}

#endif // MARK_PLUGIN_BOOKMARK_BAR_H