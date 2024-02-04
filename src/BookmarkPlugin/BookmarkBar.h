/**
   @author Kenta Suzuki
*/

#ifndef CNOID_BOOKMARK_PLUGIN_BOOKMARK_BAR_H
#define CNOID_BOOKMARK_PLUGIN_BOOKMARK_BAR_H

#include <cnoid/ExtensionManager>
#include <cnoid/ToolBar>

namespace cnoid {

class BookmarkBar : public ToolBar
{
public:
    static void initialize(ExtensionManager* ext);
    static BookmarkBar* instance();

private:
    BookmarkBar();
    virtual ~BookmarkBar();

    class Impl;
    Impl* impl;
};

}

#endif
