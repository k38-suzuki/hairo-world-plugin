/**
   @author Kenta Suzuki
*/

#include "BookmarkBar.h"
#include <cnoid/ProjectManager>
#include "BookmarkManagerDialog.h"
#include "WorldLogManagerDialog.h"
#include "gettext.h"

using namespace cnoid;
using namespace std;

namespace cnoid {

class BookmarkBar::Impl
{
public:
    BookmarkBar* self;

    Impl(BookmarkBar* self);
};

}


void BookmarkBar::initialize(ExtensionManager* ext)
{
    static bool initialized = false;
    if(!initialized) {
        ext->addToolBar(instance());
        initialized = true;
    }

    char* CNOID_USE_BOOKMARK = getenv("CNOID_USE_BOOKMARK");
    if(CNOID_USE_BOOKMARK && (strcmp(CNOID_USE_BOOKMARK, "0") == 0)){
        instance()->setVisibleByDefault(true);
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
    auto button1 = self->addButton(QIcon::fromTheme("list-add"));
    button1->setToolTip(_("Bookmark a current project"));
    button1->sigClicked().connect([&](){ 
        const string& filename = ProjectManager::instance()->currentProjectFile();
        if(!filename.empty()) {
            BookmarkManagerDialog::instance()->addProjectFile(filename);
        }
        });
    auto button2 = self->addButton(QIcon::fromTheme("user-bookmarks"));
    button2->setToolTip(_("Show the bookmark manager"));
    button2->sigClicked().connect([&](){ BookmarkManagerDialog::instance()->show(); });

    auto button3 = self->addButton(QIcon::fromTheme("emblem-documents"));
    button3->setToolTip(_("Show the worldlog manager"));
    button3->sigClicked().connect([&](){ WorldLogManagerDialog::instance()->show(); });
}


BookmarkBar::~BookmarkBar()
{
    delete impl;
}
