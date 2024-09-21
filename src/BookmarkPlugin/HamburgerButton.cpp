/**
    @author Kenta Suzuki
*/

#include "HamburgerButton.h"
#include <cnoid/Action>
#include <cnoid/ExtensionManager>
#include <cnoid/Menu>

using namespace std;
using namespace cnoid;

HamburgerButton::HamburgerButton()
{
    menu_ = new Menu;

    setIcon(QIcon(":/BookmarkPlugin/icon/bars_24.svg"));
    setMenu(menu_);
}


HamburgerButton::~HamburgerButton()
{

}


Action* HamburgerButton::addAction(const string& text)
{
    Action* action = new Action;
    action->setText(text.c_str());
    menu_->addAction(action);
    return action;
}


Menu* HamburgerButton::addMenu(const string& title)
{
    Menu* menu = new Menu(title.c_str());
    menu_->addMenu(menu);
    return menu;
}