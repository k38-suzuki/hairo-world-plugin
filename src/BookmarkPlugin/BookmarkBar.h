/**
   \file
   \author Kenta Suzuki
*/

#ifndef CNOID_BOOKMARK_PLUGIN_BOOKMARK_BAR_H
#define CNOID_BOOKMARK_PLUGIN_BOOKMARK_BAR_H

#include <cnoid/ExtensionManager>
#include <cnoid/ToolBar>

namespace cnoid {

class BookmarkBarImpl;

class BookmarkBar : public ToolBar
{
public:
    BookmarkBar();
    virtual ~BookmarkBar();

    static void initialize(ExtensionManager* ext);
    static BookmarkBar* instance();

private:
    BookmarkBarImpl* impl;
    friend class BookmarkBarImpl;
};

}

#endif // CNOID_BOOKMARK_PLUGIN_BOOKMARK_BAR_H