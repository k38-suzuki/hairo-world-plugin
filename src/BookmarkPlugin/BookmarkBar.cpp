/**
   \file
   \author Kenta Suzuki
*/

#include "BookmarkBar.h"
#include <cnoid/MainWindow>
#include <cnoid/ProjectManager>
#include <QStyle>
#include "BookmarkManagerDialog.h"
#include "WorldLogManagerDialog.h"
#include "gettext.h"

using namespace cnoid;
using namespace std;

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

    auto button1 = self->addButton(
        QIcon(MainWindow::instance()->style()->standardIcon(QStyle::SP_DialogApplyButton)));
    button1->setToolTip(_("Bookmark a current project"));
    button1->sigClicked().connect([&](){ 
        const string& filename = ProjectManager::instance()->currentProjectFile();
        if(!filename.empty()) {
            BookmarkManagerDialog::instance()->addProject(filename);
        }
        });
    auto button2 = self->addButton(
        QIcon(MainWindow::instance()->style()->standardIcon(QStyle::SP_DialogOpenButton)));
    button2->setToolTip(_("Show the bookmark manager"));
    button2->sigClicked().connect([&](){ BookmarkManagerDialog::instance()->showBookmarkManagerDialog(); });

    auto button3 = self->addButton(
        QIcon(MainWindow::instance()->style()->standardIcon(QStyle::SP_FileDialogDetailedView)));
    button3->setToolTip(_("Show the worldlog manager"));
    button3->sigClicked().connect([&](){ WorldLogManagerDialog::instance()->showWorldLogManagerDialog(); });
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