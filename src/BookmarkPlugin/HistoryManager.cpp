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

        auto action2 = get_Tools_Menu()->addAction(_("History"));
        action2->setMenu(historyInstance->contextMenu());
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

    while(this->contextMenu()->actions().size() > 10) {
            auto action = this->contextMenu()->actions().at(0);
            this->contextMenu()->removeAction(action);
    }

    for(auto& action : this->contextMenu()->actions()) {
        contextMenu->addAction(action);
    }
}