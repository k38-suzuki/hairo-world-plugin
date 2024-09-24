/**
   @author Kenta Suzuki
*/

#include "HistoryManager.h"
#include <cnoid/Action>
#include <cnoid/AppConfig>
#include <cnoid/ExtensionManager>
#include <cnoid/Menu>
#include <cnoid/MessageView>
#include <cnoid/ProjectManager>
#include <cnoid/ValueTree>
#include "HamburgerMenu.h"
#include "gettext.h"

using namespace std;
using namespace cnoid;

namespace {

HistoryManager* historyInstance = nullptr;

}


void HistoryManager::initializeClass(ExtensionManager* ext)
{
    if(!historyInstance) {
        historyInstance = ext->manage(new HistoryManager);

        const QIcon icon = QIcon(":/GoogleMaterialSymbols/icon/manage_history_24dp_5F6368_FILL1_wght400_GRAD0_opsz24.svg");
        auto action = new Action;
        action->setText(_("History Manager"));
        action->setIcon(icon);
        action->setToolTip(_("Show the history manager"));
        action->sigTriggered().connect([&](){ historyInstance->show(); });
        HamburgerMenu::instance()->addAction(action);
    }
}


HistoryManager::HistoryManager()
    : ArchiveListDialog()
{
    setWindowTitle(_("History Manager"));
    setArchiveKey("history_list");
    setFixedSize(800, 450);

    ProjectManager::instance()->sigProjectLoaded().connect([&](int level){ onProjectLoaded(level); });

    auto& recentFiles = *AppConfig::archive()->findListing("history_list");
    if(recentFiles.isValid() && !recentFiles.empty()) {
        for(int i = 0; i < recentFiles.size(); ++i) {
            if(recentFiles[i].isString()) {
                addProject(recentFiles[i].toString());
            }
        }
    }
}


HistoryManager::~HistoryManager()
{

}


void HistoryManager::onItemDoubleClicked(const string& text)
{
    ProjectManager* pm = ProjectManager::instance();
    bool result = pm->tryToCloseProject();
    if(result) {
        pm->clearProject();
        MessageView::instance()->flush();
        pm->loadProject(text);
    }
}


void HistoryManager::addProject(const string& filename)
{
    Menu* contextMenu = HamburgerMenu::instance()->contextMenu();
    if(!filename.empty()) {
        for(auto& action : contextMenu->actions()) {
            if(action->text().toStdString() == filename) {
                contextMenu->removeAction(action);
            }
        }

        if(contextMenu->actions().size() >= 16) {
            auto action = contextMenu->actions().at(0);
            contextMenu->removeAction(action);
        }

        auto action = contextMenu->addAction(filename.c_str());
        connect(action, &QAction::triggered, [this, filename](){ onItemDoubleClicked(filename); });
    }
}


void HistoryManager::onProjectLoaded(int level)
{
    string filename = ProjectManager::instance()->currentProjectFile();
    if(!filename.empty()) {
        addItem(filename.c_str());
        removeDuplicates();
        addProject(filename);
    }
}