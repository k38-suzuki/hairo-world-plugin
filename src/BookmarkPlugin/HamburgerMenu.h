/**
    @author Kenta Suzuki
*/

#ifndef CNOID_BOOKMARK_PLUGIN_HAMBURGER_MENU_H
#define CNOID_BOOKMARK_PLUGIN_HAMBURGER_MENU_H

#include <cnoid/Menu>
#include <vector>
#include "exportdecl.h"

namespace cnoid {

class ExtensionManager;
class ToolBar;

class CNOID_EXPORT HamburgerMenu : public Menu
{
public:
    static void initializeClass(ExtensionManager* ext);
    static HamburgerMenu* instance();

    HamburgerMenu(QWidget* parent = nullptr);
    HamburgerMenu(const QString& title, QWidget* parent = nullptr);
    virtual ~HamburgerMenu();

    Menu* contextMenu() { return contextMenu_; }

private:
    void initialize();

    Menu* contextMenu_;
};

CNOID_EXPORT Menu* get_Tools_Menu();
CNOID_EXPORT ToolBar* fileBar();
CNOID_EXPORT bool loadProject(const std::string& filename);
CNOID_EXPORT std::string getSaveFileName(const std::string& caption, const std::string& extensions);
CNOID_EXPORT std::vector<std::string> getSaveFileNames(const std::string& caption, const std::string& extensions);

}

#endif // CNOID_BOOKMARK_PLUGIN_HAMBURGER_MENU_H