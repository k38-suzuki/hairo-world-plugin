/**
   @author Kenta Suzuki
*/

#include "HistoryManager.h"
#include <cnoid/ExtensionManager>
#include <cnoid/MainWindow>
#include <cnoid/MessageView>
#include <cnoid/ProjectManager>
#include <cnoid/ToolBar>
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

        vector<ToolBar*> toolBars = MainWindow::instance()->toolBars();
        for(auto& bar : toolBars) {
            if(bar->name() == "FileBar") {
                auto button1 = bar->addButton(QIcon::fromTheme("document-revert"));
                button1->setToolTip(_("Show the history manager"));
                button1->sigClicked().connect([&](){
                    historyInstance->updateList();
                    historyInstance->show(); });
            }
        }       
    }
}


HistoryManager::HistoryManager()
{
    setWindowTitle(_("History Manager"));
    setArchiveKey("history_list");
    setFixedSize(800, 450);

    ProjectManager::instance()->sigProjectLoaded().connect(
        [&](int level){ addItem(ProjectManager::instance()->currentProjectFile().c_str()); });
}


HistoryManager::~HistoryManager()
{

}


void HistoryManager::onItemDoubleClicked(const string& text)
{
    if(ProjectManager::instance()->tryToCloseProject()) {
        ProjectManager::instance()->clearProject();
        MessageView::instance()->flush();
        ProjectManager::instance()->loadProject(text);
    }
}
