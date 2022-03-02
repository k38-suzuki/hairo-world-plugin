/**
   \file
   \author Kenta Suzuki
*/

#include "BookmarkList.h"
#include <cnoid/Action>
#include <cnoid/AppConfig>
#include <cnoid/Menu>
#include <cnoid/MenuManager>
#include <cnoid/ProjectManager>
#include "gettext.h"

using namespace cnoid;
using namespace std;

namespace cnoid {

class BookmarkListImpl
{
public:
    BookmarkListImpl(BookmarkList* self, ExtensionManager* ext);
    virtual ~BookmarkListImpl();
    BookmarkList* self;

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


BookmarkList::BookmarkList(ExtensionManager* ext)
{
    impl = new BookmarkListImpl(this, ext);
}


BookmarkListImpl::BookmarkListImpl(BookmarkList* self, ExtensionManager* ext)
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

    QObject::connect(currentMenu, &Menu::customContextMenuRequested, [=](const QPoint& pos){ onCustomContextMenuRequested(pos); });

    Mapping& config = *AppConfig::archive()->openMapping("bookmark");
    if(config.isValid()) {
        restore(config);
    }
}


BookmarkList::~BookmarkList()
{
    delete impl;
}


BookmarkListImpl::~BookmarkListImpl()
{
    store(*AppConfig::archive()->openMapping("bookmark"));
}


void BookmarkList::initialize(ExtensionManager* ext)
{
    ext->manage(new BookmarkList(ext));
}


void BookmarkListImpl::onAddProjectTriggered()
{
    const string& filename = pm->currentProjectFile();
    if(!filename.empty()) {
        Action* currentProject = new Action;
        currentProject->setText(filename.c_str());
        currentMenu->addAction(currentProject);
    }
}


void BookmarkListImpl::onRemoveProjectTriggered()
{
    currentMenu->removeAction(currentMenu->actionAt(pos));
}


void BookmarkListImpl::onCurrentMenuTriggered(QAction* action)
{
    Action* triggeredProject = dynamic_cast<Action*>(action);
    if(triggeredProject != addProject) {
        pm->loadProject(triggeredProject->text().toStdString());
    }
}


void BookmarkListImpl::onCustomContextMenuRequested(const QPoint& pos)
{
    this->pos = pos;
    if(currentMenu->actionAt(this->pos) != addProject) {
        contextMenu->exec(currentMenu->mapToGlobal(pos));
    }
}


void BookmarkListImpl::store(Mapping& archive)
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


void BookmarkListImpl::restore(const Mapping& archive)
{
    int numBookmarks = archive.get("num_bookmarks", 0);
    for(int i = 0; i < numBookmarks; ++i) {
        string key = "bookmark_" + to_string(i);
        string filename = archive.get(key, "");
        Action* action = new Action();
        action->setText(filename.c_str());
        currentMenu->addAction(action);
    }
}
