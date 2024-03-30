/**
   @author Kenta Suzuki
*/

#ifndef CNOID_BOOKMARK_PLUGIN_TOOLS_MENU_H
#define CNOID_BOOKMARK_PLUGIN_TOOLS_MENU_H

#include <QAction>
#include <QMenu>

namespace cnoid {

class ExtensionManager;

class ToolsMenu
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

QMenu* toolsMenu();

}

#endif // CNOID_BOOKMARK_PLUGIN_TOOLS_MENU_H
