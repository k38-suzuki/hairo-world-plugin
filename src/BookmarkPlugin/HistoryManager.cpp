/**
   @author Kenta Suzuki
*/

#include "HistoryManager.h"
#include <cnoid/Action>
#include <cnoid/AppConfig>
#include <cnoid/Archive>
#include <cnoid/ExtensionManager>
#include <cnoid/Menu>
#include <cnoid/MenuManager>
#include <cnoid/MessageView>
#include <cnoid/ProjectManager>
#include "gettext.h"

using namespace cnoid;
using namespace std;

namespace cnoid {

class HistoryManager::Impl
{
public:
    HistoryManager* self;

    Impl(HistoryManager* self, ExtensionManager* ext);
    ~Impl();

    Menu* currentMenu;
    Menu* contextMenu;
    Action* clearProject;
    ProjectManager* pm;
    QPoint pos;

    void onClearProjectTriggered();
    void onRemoveProjectTriggered();
    void onCurrentMenuTriggered(QAction* action);
    void onCustomContextMenuRequested(const QPoint& pos);
    void onProjectLoaded();
};

}


HistoryManager::HistoryManager(ExtensionManager* ext)
{
    impl = new Impl(this, ext);
}


HistoryManager::Impl::Impl(HistoryManager* self, ExtensionManager* ext)
    : self(self),
      pm(ProjectManager::instance())
{
    MenuManager& mm = ext->menuManager().setPath("/" N_("Tools")).setPath(_("History"));
    currentMenu = mm.currentMenu();
    currentMenu->setContextMenuPolicy(Qt::CustomContextMenu);
    clearProject = new Action;
    clearProject->setText(_("Clear all histories"));
    clearProject->sigTriggered().connect([&](){ onClearProjectTriggered(); });
    currentMenu->addAction(clearProject);
    currentMenu->addSeparator();
    currentMenu->sigTriggered().connect([&](QAction* action){ onCurrentMenuTriggered(action); });

    contextMenu = new Menu;
    Action* removeProject = new Action;
    removeProject->setText(_("Remove"));
    removeProject->sigTriggered().connect([&](){ onRemoveProjectTriggered(); });
    contextMenu->addAction(removeProject);

    QObject::connect(currentMenu, &Menu::customContextMenuRequested,
        [=](const QPoint& pos){ onCustomContextMenuRequested(pos); });

    pm->sigProjectLoaded().connect([&](int level){ onProjectLoaded(); });

    // restore config
    auto& historyList = *AppConfig::archive()->findListing("histories");
    if(historyList.isValid() && !historyList.empty()) {
        for(int i = 0; i < historyList.size(); ++i) {
            if(historyList[i].isString()) {
                string filename = historyList[i].toString();
                if(!filename.empty()) {
                    Action* action = new Action;
                    action->setText(filename.c_str());
                    currentMenu->addAction(action);
                }
            }
        }
    }
}


HistoryManager::~HistoryManager()
{
    delete impl;
}


HistoryManager::Impl::~Impl()
{
    // store config
    int numHistories = currentMenu->actions().size() - 2;

    ListingPtr historyList = new Listing;
    historyList->append(ProjectManager::instance()->currentProjectFile(), DOUBLE_QUOTED);
    for(int i = 0; i < numHistories; ++i) {
        QAction* action = currentMenu->actions()[i + 2];
        string filename = action->text().toStdString();
        historyList->append(filename, DOUBLE_QUOTED);
    }
    AppConfig::archive()->insert("histories", historyList);
}


void HistoryManager::initializeClass(ExtensionManager* ext)
{
    ext->manage(new HistoryManager(ext));
}


void HistoryManager::Impl::onClearProjectTriggered()
{
    int numHistories = currentMenu->actions().size() - 2;
    for(int i = 0; i < numHistories; ++i) {
        currentMenu->removeAction(currentMenu->actions()[2]);
    }
}


void HistoryManager::Impl::onRemoveProjectTriggered()
{
    currentMenu->removeAction(currentMenu->actionAt(pos));
}


void HistoryManager::Impl::onCurrentMenuTriggered(QAction* action)
{
    Action* triggeredProject = dynamic_cast<Action*>(action);
    if(triggeredProject != clearProject) {
        bool result = pm->tryToCloseProject();
        if(result) {
            pm->clearProject();
            MessageView::instance()->flush();
            pm->loadProject(action->text().toStdString());
        }
    }
}


void HistoryManager::Impl::onCustomContextMenuRequested(const QPoint& pos)
{
    this->pos = pos;
    if(currentMenu->actionAt(this->pos) != clearProject) {
        contextMenu->exec(currentMenu->mapToGlobal(pos));
    }
}


void HistoryManager::Impl::onProjectLoaded()
{
    string filename = pm->currentProjectFile();

    if(!filename.empty()) {
        Action* action = new Action;
        action->setText(filename.c_str());
        currentMenu->addAction(action);

        int maxHistory = 10;
        int numHistories = currentMenu->actions().size();
        if(numHistories > maxHistory) {
            currentMenu->removeAction(currentMenu->actions()[0]);
        }
    }
}
