/**
   \file
   \author Kenta Suzuki
*/

#include "BookmarkManager.h"
#include <cnoid/Action>
#include <cnoid/AppConfig>
#include <cnoid/Menu>
#include <cnoid/MenuManager>
#include <cnoid/MessageView>
#include <cnoid/ProjectManager>
#include "gettext.h"

using namespace cnoid;
using namespace std;

namespace cnoid {

class BookmarkManagerImpl
{
public:
    BookmarkManagerImpl(BookmarkManager* self, ExtensionManager* ext);
    virtual ~BookmarkManagerImpl();
    BookmarkManager* self;

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


BookmarkManager::BookmarkManager(ExtensionManager* ext)
{
    impl = new BookmarkManagerImpl(this, ext);
}


BookmarkManagerImpl::BookmarkManagerImpl(BookmarkManager* self, ExtensionManager* ext)
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


BookmarkManager::~BookmarkManager()
{
    delete impl;
}


BookmarkManagerImpl::~BookmarkManagerImpl()
{
    store(*AppConfig::archive()->openMapping("bookmark"));
}


void BookmarkManager::initialize(ExtensionManager* ext)
{
    ext->manage(new BookmarkManager(ext));
}


void BookmarkManagerImpl::onAddProjectTriggered()
{
    const string& filename = pm->currentProjectFile();
    if(!filename.empty()) {
        Action* currentProject = new Action;
        currentProject->setText(filename.c_str());
        currentMenu->addAction(currentProject);
    }
}


void BookmarkManagerImpl::onRemoveProjectTriggered()
{
    currentMenu->removeAction(currentMenu->actionAt(pos));
}


void BookmarkManagerImpl::onCurrentMenuTriggered(QAction* action)
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


void BookmarkManagerImpl::onCustomContextMenuRequested(const QPoint& pos)
{
    this->pos = pos;
    if(currentMenu->actionAt(this->pos) != addProject) {
        contextMenu->exec(currentMenu->mapToGlobal(pos));
    }
}


void BookmarkManagerImpl::store(Mapping& archive)
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


void BookmarkManagerImpl::restore(const Mapping& archive)
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
