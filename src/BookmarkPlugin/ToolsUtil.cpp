/**
   @author Kenta Suzuki
*/

#include "ToolsUtil.h"
#include <cnoid/Action>
#include <cnoid/ExtensionManager>
#include <cnoid/MainWindow>
#include <cnoid/Menu>
#include <cnoid/MenuManager>
#include "gettext.h"

using namespace std;
using namespace cnoid;

namespace {

Menu* menu_Tools = nullptr;
ToolBar* fileBar_ = nullptr;

}

void ToolsMenu::initializeClass(ExtensionManager* ext)
{
    if(!menu_Tools) {
        MenuManager& mm = ext->menuManager().setPath("/" N_("Tools"));
        menu_Tools = mm.currentMenu();
    }
}


ToolsMenu::ToolsMenu(const string& text)
{
    currentMenu = menu_Tools->addMenu(text.c_str());
    max_actions_ = 16;
}


ToolsMenu::~ToolsMenu()
{

}


QAction* ToolsMenu::addAction(const string& text)
{
    if(currentMenu->actions().size() == max_actions_) {
        QAction* action = currentMenu->actions()[0];
        currentMenu->removeAction(action);
    }
    return currentMenu->addAction(text.c_str());
}


QAction* ToolsMenu::action(int actionId)
{
    return currentMenu->actions()[actionId];
}


void ToolsMenu::clearActions()
{
    for(auto& action : currentMenu->actions()) {
        currentMenu->removeAction(action);
    }
}


namespace cnoid {

QMenu* toolsMenu()
{
    return menu_Tools;
}


ToolBar* fileBar()
{
    if(!fileBar_) {
        vector<ToolBar*> toolBars = MainWindow::instance()->toolBars();
        for(auto& toolBar : toolBars) {
            if(toolBar->name() == "FileBar") {
                fileBar_ = toolBar;
            }
        }
    }
    return fileBar_;
}

}
