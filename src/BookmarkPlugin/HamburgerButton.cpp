/**
    @author Kenta Suzuki
*/

#include "HamburgerButton.h"
#include <cnoid/Action>
#include <cnoid/ExtensionManager>
#include <cnoid/Menu>

using namespace std;
using namespace cnoid;

HamburgerButton::HamburgerButton(QWidget* parent)
    : PushButton(parent)
{
    initialize();
}


HamburgerButton::HamburgerButton(const QString& text, QWidget* parent)
    : PushButton(text, parent)
{
    initialize();
}


HamburgerButton::HamburgerButton(const QIcon& icon, const QString& text, QWidget* parent)
    : PushButton(icon, text, parent)
{
    initialize();
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


void HamburgerButton::initialize()
{
    menu_ = new Menu;
    setIcon(QIcon(":/GoogleMaterialSymbols/icon/menu_24dp_5F6368.svg"));
    setMenu(menu_);
}