/**
    @author Kenta Suzuki
*/

#include "HamburgerMenu.h"
#include <cnoid/Action>
#include <cnoid/ExtensionManager>
#include <cnoid/Menu>
#include <cnoid/ToolsUtil>

using namespace std;
using namespace cnoid;

namespace {

HamburgerMenu* hamburgerInstance = nullptr;

}

namespace cnoid {

class HamburgerMenu::Impl
{
public:

    Impl();
    ~Impl();

    QMenu* menu;
};

}


void HamburgerMenu::initializeClass(ExtensionManager* ext)
{
    if(!hamburgerInstance) {
        hamburgerInstance = ext->manage(new HamburgerMenu);
    }
}


HamburgerMenu* HamburgerMenu::instance()
{
    return hamburgerInstance;
}


HamburgerMenu::HamburgerMenu()
{
    impl = new Impl;
}


HamburgerMenu::Impl::Impl()
{
    menu = new Menu;
    menu->addAction(action);

    auto button = fileBar()->addButton(":/BookmarkPlugin/icon/bars_hoso.svg");
    button->setMenu(menu);
}


HamburgerMenu::~HamburgerMenu()
{
    delete impl;
}


HamburgerMenu::Impl::~Impl()
{

}


Action* HamburgerMenu::addAction(const string& text)
{
    Action* action = new Action;
    action->setText(text.c_str());
    impl->menu->addAction(action);
    return action;
}


Menu* HamburgerMenu::addMenu(const string& title)
{
    Menu* menu = new Menu(title.c_str());
    impl->menu->addMenu(menu);
    return menu;
}