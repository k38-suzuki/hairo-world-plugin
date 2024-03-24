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

using namespace std;
using namespace cnoid;

namespace cnoid {

class HistoryManager::Impl
{
public:
    HistoryManager* self;

    enum { MaxFiles = 8 };

    Impl(HistoryManager* self, ExtensionManager* ext);
    ~Impl();

    Menu* currentMenu;
    Menu* contextMenu;
    QPoint pos;

    void onRemoveProjectTriggered();
    void onCurrentMenuTriggered(QAction* action);
    void onCustomContextMenuRequested(const QPoint& pos);
    void onProjectLoaded();
    void updatePresetFiles();
    void storeRecentFiles();
};

}


HistoryManager::HistoryManager(ExtensionManager* ext)
{
    impl = new Impl(this, ext);
}


HistoryManager::Impl::Impl(HistoryManager* self, ExtensionManager* ext)
    : self(self)
{
    MenuManager& mm = ext->menuManager().setPath("/" N_("Tools")).setPath(_("History"));
    currentMenu = mm.currentMenu();
    currentMenu->setContextMenuPolicy(Qt::CustomContextMenu);
    currentMenu->sigTriggered().connect([&](QAction* action){ onCurrentMenuTriggered(action); });

    contextMenu = new Menu;
    Action* removeProject = new Action;
    removeProject->setText(_("Remove"));
    removeProject->sigTriggered().connect([&](){ onRemoveProjectTriggered(); });
    contextMenu->addAction(removeProject);

    QObject::connect(currentMenu, &Menu::customContextMenuRequested,
        [=](const QPoint& pos){ onCustomContextMenuRequested(pos); });

    ProjectManager::instance()->sigProjectLoaded().connect(
        [&](int level){ onProjectLoaded(); });

    updatePresetFiles();
}


HistoryManager::~HistoryManager()
{
    delete impl;
}


HistoryManager::Impl::~Impl()
{
    ListingPtr historyList = new Listing;
    historyList->append(ProjectManager::instance()->currentProjectFile(), DOUBLE_QUOTED);
    for(auto& action : currentMenu->actions()) {
        string filename = action->text().toStdString();
        historyList->append(filename, DOUBLE_QUOTED);
    }
    AppConfig::archive()->insert("histories", historyList);
}


void HistoryManager::initializeClass(ExtensionManager* ext)
{
    ext->manage(new HistoryManager(ext));
}


void HistoryManager::Impl::onRemoveProjectTriggered()
{
    currentMenu->removeAction(currentMenu->actionAt(pos));
}


void HistoryManager::Impl::onCurrentMenuTriggered(QAction* action)
{
    if(ProjectManager::instance()->tryToCloseProject()) {
        ProjectManager::instance()->clearProject();
        MessageView::instance()->flush();
        ProjectManager::instance()->loadProject(action->text().toStdString());
    }
}


void HistoryManager::Impl::onCustomContextMenuRequested(const QPoint& pos)
{
    this->pos = pos;
    contextMenu->exec(currentMenu->mapToGlobal(pos));
}


void HistoryManager::Impl::onProjectLoaded()
{
    string filename = ProjectManager::instance()->currentProjectFile();

    if(!filename.empty()) {
        bool is_duplicated = false;
        for(auto& action : currentMenu->actions()) {
            string history = action->text().toStdString();
            if(history == filename) {
                is_duplicated = true;
            }
        }

        if(!is_duplicated) {
            Action* action = new Action;
            action->setText(filename.c_str());
            currentMenu->addAction(action);
        }

        if(currentMenu->actions().size() > MaxFiles) {
            currentMenu->removeAction(currentMenu->actions()[0]);
        }
    }
}


void HistoryManager::Impl::updatePresetFiles()
{
    QStringList qFile;
    auto& recentFiles = *AppConfig::archive()->findListing("histories");
    if(recentFiles.isValid() && !recentFiles.empty()) {
        for(int i = recentFiles.size() - 1; i >= 0; --i) {
            if(recentFiles[i].isString()) {
                qFile << recentFiles[i].toString().c_str();
                auto file = recentFiles[i].toString();
                if(!file.empty()) {
                    Action* action = new Action;
                    action->setText(file);
                    currentMenu->addAction(action);
                }
            }
        }
    }
}


void HistoryManager::Impl::storeRecentFiles()
{
    // string latestFile = ProjectManager::instance()->currentProjectFile();
    // ListingPtr recentFiles = new Listing;
    // recentFiles->append(latestFile, DOUBLE_QUOTED);
    // auto oldRecentFiles = AppConfig::archive()->openListing("histories");
    // for(auto& node : *oldRecentFiles) {
    //     auto file = node->toString();
    //     if(file != latestFile) {
    //         recentFiles->append(file, DOUBLE_QUOTED);   
    //         if(recentFiles->size() >= MaxFiles) {
    //             break;
    //         }
    //     }
    //     AppConfig::archive()->insert("histories", recentFiles);
    // }
}
