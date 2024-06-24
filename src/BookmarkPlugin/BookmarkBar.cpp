/**
   @author Kenta Suzuki
*/

#include "BookmarkBar.h"
#include <cnoid/ExtensionManager>
#include "gettext.h"

using namespace std;
using namespace cnoid;

namespace cnoid {

class BookmarkBar::Impl
{
public:
    BookmarkBar* self;

    Impl(BookmarkBar* self);
    ~Impl();
};

}


void BookmarkBar::initialize(ExtensionManager* ext)
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


BookmarkBar::BookmarkBar()
    : ToolBar(N_("BookmarkBar"))
{
    impl = new Impl(this);
}


BookmarkBar::Impl::Impl(BookmarkBar* self)
    : self(self)
{
    self->setVisibleByDefault(false);
}


BookmarkBar::~BookmarkBar()
{
    delete impl;
}


BookmarkBar::Impl::~Impl()
{

}