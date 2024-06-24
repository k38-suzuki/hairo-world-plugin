/**
   @author Kenta Suzuki
*/

#ifndef CNOID_BOOKMARK_PLUGIN_BOOKMARK_BAR_H
#define CNOID_BOOKMARK_PLUGIN_BOOKMARK_BAR_H

#include <cnoid/ToolBar>
#include "exportdecl.h"

namespace cnoid {

class ExtensionManager;

class CNOID_EXPORT BookmarkBar : public ToolBar
{
public:
    static void initialize(ExtensionManager* ext);
    static BookmarkBar* instance();

private:
    class Impl;
    Impl* impl;

    BookmarkBar();
    ~BookmarkBar();
};

}

#endif // CNOID_BOOKMARK_PLUGIN_BOOKMARK_BAR_H