/**
   @author Kenta Suzuki
*/

#ifndef CNOID_BOOKMARK_PLUGIN_TOOLS_UTIL_H
#define CNOID_BOOKMARK_PLUGIN_TOOLS_UTIL_H

#include <cnoid/ToolBar>
#include <QAction>
#include <QMenu>
#include "exportdecl.h"

namespace cnoid {

class ExtensionManager;

class CNOID_EXPORT ToolsMenu
{
public:
    static void initializeClass(ExtensionManager* ext);

    ToolsMenu(const std::string& text);
    virtual ~ToolsMenu();

    QAction* addAction(const std::string& text);
    QAction* action(int actionId);
    void clearActions();
    void setMaxActions(const int& max_actions) { max_actions_ = max_actions; }

private:
    QMenu* currentMenu;
    int max_actions_;
};

CNOID_EXPORT QMenu* toolsMenu();
CNOID_EXPORT CNOID_EXPORT ToolBar* fileBar();

}

#endif // CNOID_BOOKMARK_PLUGIN_TOOLS_UTIL_H
