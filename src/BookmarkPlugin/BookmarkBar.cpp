/**
   \file
   \author Kenta Suzuki
*/

#include "BookmarkBar.h"
#include "gettext.h"

using namespace cnoid;

namespace cnoid {

class BookmarkBarImpl
{
public:
    BookmarkBarImpl(BookmarkBar* self);
    BookmarkBar* self;
};

}


BookmarkBar::BookmarkBar()
    : ToolBar(N_("BookmarkBar"))
{
    impl = new BookmarkBarImpl(this);
}


BookmarkBarImpl::BookmarkBarImpl(BookmarkBar* self)
    : self(self)
{
    self->setVisibleByDefault(true);
}


BookmarkBar::~BookmarkBar()
{
    delete impl;
}


void BookmarkBar::initializeClass(ExtensionManager* ext)
{
    static bool initialized = false;
    if(!initialized) {
        ext->addToolBar(instance());
        initialized = true;
    }
}


BookmarkBar* BookmarkBar::instance()
{
    static BookmarkBar* bookmarkBar = new BookmarkBar;
    return bookmarkBar;
}