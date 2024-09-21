/**
   @author Kenta Suzuki
*/

#include "BookmarkBar.h"
#include <cnoid/AppConfig>
#include <cnoid/Action>
#include <cnoid/ExtensionManager>
#include <cnoid/Menu>
#include <cnoid/ProjectManager>
#include <cnoid/ValueTree>
#include <QInputDialog>
#include <vector>
#include "gettext.h"

using namespace std;
using namespace cnoid;

namespace {

struct BookmarkInfo {
    string label;
    string filename;
};

}

namespace cnoid {

class BookmarkBar::Impl
{
public:
    BookmarkBar* self;

    Impl(BookmarkBar* self);
    ~Impl();

    void onRemoveActionTriggered();
    void onBookmarkButtonClicked(const string& filename);
    void onInputButtonClicked();
    void onCustomContextMenuRequested(const QPoint& pos);

    Action* removeAction;
    Menu* menu;
    ToolButton* targetButton;

    vector<BookmarkInfo> bookmarks;
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

    menu = new Menu;
    targetButton = nullptr;
    bookmarks.clear();

    removeAction = new Action;
    removeAction->setText(_("Remove"));
    removeAction->sigTriggered().connect([&](){ onRemoveActionTriggered(); });

    auto button = self->addButton(":/BookmarkPlugin/icon/bookmark_add_24dp_5F6368.svg");
    // button->setMenu(menu);
    button->sigClicked().connect([&](){ onInputButtonClicked(); });
}


BookmarkBar::~BookmarkBar()
{
    delete impl;
}


BookmarkBar::Impl::~Impl()
{

}


void BookmarkBar::Impl::onRemoveActionTriggered()
{
    if(targetButton) {
        delete targetButton;
    }
}


void BookmarkBar::Impl::onBookmarkButtonClicked(const string& filename)
{
    if(ProjectManager::instance()->tryToCloseProject()) {
        ProjectManager::instance()->loadProject(filename);
    }
}


void BookmarkBar::Impl::onInputButtonClicked()
{
    string project_name = ProjectManager::instance()->currentProjectName();
    string project_file = ProjectManager::instance()->currentProjectFile();

    bool ok;
    QString text = QInputDialog::getText(self, _("Input label"),
        _("Label:"), QLineEdit::Normal, project_name.c_str(), &ok);

    if(ok && !text.isEmpty()) {
        string label = text.toStdString();
        auto button = self->addButton(label.c_str());
        button->sigClicked().connect(
            [&, project_file](){ onBookmarkButtonClicked(project_file); });

        button->setContextMenuPolicy(Qt::CustomContextMenu);
        self->connect(button, &ToolButton::customContextMenuRequested,
            [&, button](const QPoint& pos){
                targetButton = button;
                onCustomContextMenuRequested(pos);
            });

        BookmarkInfo info = { label, project_file };
        bookmarks.push_back(info);
    }
}


void BookmarkBar::Impl::onCustomContextMenuRequested(const QPoint& pos)
{
    Menu menu;
    menu.addAction(removeAction);
    menu.exec(self->mapToGlobal(pos));
}