/**
   @author Kenta Suzuki
*/

#include "HistoryManager.h"
#include <cnoid/Action>
#include <cnoid/ExtensionManager>
#include <cnoid/Menu>
#include <cnoid/ProjectManager>
#include "HamburgerMenu.h"
#include "ProjectListedDialog.h"
#include "ListedWidget.h"
#include "gettext.h"

using namespace std;
using namespace cnoid;


void HistoryManager::initializeClass(ExtensionManager* ext)
{
    static HistoryManager* widget = nullptr;

    if(!widget) {
        widget = ext->manage(new HistoryManager);

        ProjectListedDialog::instance()->listedWidget()->addWidget(_("History"), widget);

        auto action = get_Tools_Menu()->addAction(_("History"));
        action->setMenu(widget->contextMenu());
    }
}


HistoryManager::HistoryManager(QWidget* parent)
    : ArchiveListWidget(parent)
{
    setWindowTitle(_("History Manager"));
    setArchiveKey("history_list");
    setMinimumSize(640, 480);

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