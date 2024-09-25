/**
   @author Kenta Suzuki
*/

#include "HistoryManager.h"
#include <cnoid/Action>
#include <cnoid/ExtensionManager>
#include <cnoid/Menu>
#include <cnoid/ProjectManager>
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


HistoryManager::HistoryManager(QWidget* parent)
    : ArchiveListDialog(parent)
{
    setWindowTitle(_("History Manager"));
    setArchiveKey("history_list");
    setFixedSize(800, 450);

    ProjectManager::instance()->sigProjectLoaded().connect(
        [&](int level){ onProjectLoaded(level); });

    clampActions();
    this->sigListUpdated().connect([&](){ clampActions(); });
}


HistoryManager::~HistoryManager()
{

}


void HistoryManager::onItemDoubleClicked(const string& text)
{
    if(loadProject(text)) {

    }
}


void HistoryManager::onProjectLoaded(int level)
{
    string filename = ProjectManager::instance()->currentProjectFile();
    if(!filename.empty()) {
        addItem(filename.c_str());
        removeDuplicates();
    }
}


void HistoryManager::clampActions()
{
    Menu* contextMenu = HamburgerMenu::instance()->contextMenu();
    contextMenu->clear();

    for(auto& action : this->contextMenu()->actions()) {
        if(contextMenu->actions().size() >= 16) {
            auto action2 = contextMenu->actions().at(0);
            contextMenu->removeAction(action2);
        }
        contextMenu->addAction(action);
    }
}