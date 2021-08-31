/**
   \file
   \author Kenta Suzuki
*/

#include "BookmarkBar.h"
#include <cnoid/Button>
#include "BookmarkManagerDialog.h"
#include "gettext.h"

using namespace cnoid;

BookmarkBar* bookmarkBar = nullptr;

namespace cnoid {

class BookmarkBarImpl
{
public:
    BookmarkBarImpl(BookmarkBar* self);
    BookmarkBar* self;

    BookmarkManagerDialog* dialog;
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
//    self->setVisibleByDefault(true);

    dialog = new BookmarkManagerDialog();
    self->addButton(QIcon(":/Bookmark/icon/bookmark.svg"), _("Show the bookmark dialog"))
            ->sigClicked().connect([&](){ dialog->show(); });
}


BookmarkBar::~BookmarkBar()
{
    delete impl;
}


void BookmarkBar::initializeClass(ExtensionManager* ext)
{
    if(!bookmarkBar) {
        bookmarkBar = new BookmarkBar();
        ext->addToolBar(bookmarkBar);
    }
}


BookmarkBar* BookmarkBar::instance()
{
    return bookmarkBar;
}
