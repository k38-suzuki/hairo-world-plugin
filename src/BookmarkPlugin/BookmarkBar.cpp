/**
   \file
   \author Kenta Suzuki
*/

#include "BookmarkBar.h"
#include <cnoid/Action>
#include <cnoid/AppConfig>
#include <cnoid/Button>
#include <cnoid/MainWindow>
#include <cnoid/Menu>
#include <cnoid/MenuManager>
#include <cnoid/MessageView>
#include <cnoid/ProjectManager>
#include <cnoid/BookmarkManager>
#include <QStyle>
#include "gettext.h"

using namespace cnoid;
using namespace std;

namespace {

BookmarkBar* bookmarkBar;

}

namespace cnoid {

class BookmarkBarImpl
{
public:
    BookmarkBarImpl(BookmarkBar* self, ExtensionManager* ext);
    virtual ~BookmarkBarImpl();
    BookmarkBar* self;

    Menu* currentMenu;
    Menu* contextMenu;
    Action* addProject;
    ProjectManager* pm;
    QPoint pos;

    void onAddProjectTriggered();
    void onRemoveProjectTriggered();
    void onCurrentMenuTriggered(QAction* action);
    void onCustomContextMenuRequested(const QPoint& pos);
    void store(Mapping& archive);
    void restore(const Mapping& archive);
};

}


BookmarkBar::BookmarkBar(ExtensionManager* ext)
    : ToolBar(N_("BookmarkBar"))
{
    impl = new BookmarkBarImpl(this, ext);
}


BookmarkBarImpl::BookmarkBarImpl(BookmarkBar* self, ExtensionManager* ext)
    : self(self),
      pm(ProjectManager::instance())
{
    MenuManager& mm = ext->menuManager().setPath("/" N_("Bookmark"));
    currentMenu = mm.currentMenu();
    currentMenu->setContextMenuPolicy(Qt::CustomContextMenu);
    addProject = new Action;
    addProject->setText(_("Add current project"));
    addProject->sigTriggered().connect([&](){ onAddProjectTriggered(); });
    currentMenu->addAction(addProject);
    currentMenu->addSeparator();
    currentMenu->sigTriggered().connect([&](QAction* action){ onCurrentMenuTriggered(action); });

    contextMenu = new Menu;
    Action* removeProject = new Action;
    removeProject->setText(_("Remove"));
    removeProject->sigTriggered().connect([&](){ onRemoveProjectTriggered(); });
    contextMenu->addAction(removeProject);

    self->connect(currentMenu, &Menu::customContextMenuRequested,
        [=](const QPoint& pos){ onCustomContextMenuRequested(pos); });

    auto button1 = self->addButton(
        QIcon(MainWindow::instance()->style()->standardIcon(QStyle::SP_DialogApplyButton)));
    button1->sigClicked().connect([&](){  });
    auto button2 = self->addButton(
        QIcon(MainWindow::instance()->style()->standardIcon(QStyle::SP_FileDialogDetailedView)));
    button2->sigClicked().connect([&](){ BookmarkManager::instance()->showBookmarkManagerDialog(); });

    Mapping& config = *AppConfig::archive()->openMapping("bookmark_bar");
    if(config.isValid()) {
        restore(config);
    }
}


BookmarkBar::~BookmarkBar()
{
    delete impl;
}


BookmarkBarImpl::~BookmarkBarImpl()
{
    store(*AppConfig::archive()->openMapping("bookmark_bar"));
}


void BookmarkBar::initializeClass(ExtensionManager* ext)
{
    if(!bookmarkBar) {
        bookmarkBar = new BookmarkBar(ext);
        ext->addToolBar(bookmarkBar);
    }
}


BookmarkBar* BookmarkBar::instance()
{
    return bookmarkBar;
}


void BookmarkBarImpl::onAddProjectTriggered()
{
    const string& filename = pm->currentProjectFile();
    if(!filename.empty()) {
        Action* currentProject = new Action;
        currentProject->setText(filename.c_str());
        currentMenu->addAction(currentProject);
    }
}


void BookmarkBarImpl::onRemoveProjectTriggered()
{
    currentMenu->removeAction(currentMenu->actionAt(pos));
}


void BookmarkBarImpl::onCurrentMenuTriggered(QAction* action)
{
    Action* triggeredProject = dynamic_cast<Action*>(action);
    if(triggeredProject != addProject) {
        bool result = pm->tryToCloseProject();
        if(result) {
            pm->clearProject();
            MessageView::instance()->flush();
            pm->loadProject(triggeredProject->text().toStdString());
        }
    }
}


void BookmarkBarImpl::onCustomContextMenuRequested(const QPoint& pos)
{
    this->pos = pos;
    if(currentMenu->actionAt(this->pos) != addProject) {
        contextMenu->exec(currentMenu->mapToGlobal(pos));
    }
}


void BookmarkBarImpl::store(Mapping& archive)
{
    int numBookmarks = currentMenu->actions().size() - 2;
    archive.write("num_bookmarks", numBookmarks);
    for(int i = 0; i < numBookmarks; ++i) {
        QAction* action = currentMenu->actions()[i + 2];
        string key = "bookmark_" + to_string(i);
        string filename = action->text().toStdString();
        archive.write(key, filename);
    }
}


void BookmarkBarImpl::restore(const Mapping& archive)
{
    int numBookmarks = archive.get("num_bookmarks", 0);
    for(int i = 0; i < numBookmarks; ++i) {
        string key = "bookmark_" + to_string(i);
        string filename = archive.get(key, "");
        Action* action = new Action;
        action->setText(filename.c_str());
        currentMenu->addAction(action);
    }
}
