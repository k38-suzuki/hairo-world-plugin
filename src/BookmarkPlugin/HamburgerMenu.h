/**
    @author Kenta Suzuki
*/

#ifndef CNOID_BOOKMARK_PLUGIN_HAMBURGER_MENU_H
#define CNOID_BOOKMARK_PLUGIN_HAMBURGER_MENU_H

#include <string>
#include "exportdecl.h"

namespace cnoid {

class Action;
class ExtensionManager;
class Menu;

class CNOID_EXPORT HamburgerMenu
{
public:
    static void initializeClass(ExtensionManager* ext);
    static HamburgerMenu* instance();

    HamburgerMenu();;
    virtual ~HamburgerMenu();

    Action* addAction(const std::string& text);
    Menu* addMenu(const std::string& title);

private:
    class Impl;
    Impl* impl;
};

}

#endif // CNOID_BOOKMARK_PLUGIN_HAMBURGER_MENU_H