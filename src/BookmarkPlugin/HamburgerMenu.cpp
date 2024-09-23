/**
    @author Kenta Suzuki
*/

#include "HamburgerMenu.h"
#include <cnoid/ExtensionManager>
#include <cnoid/MainWindow>
#include <cnoid/Menu>
#include <cnoid/MenuManager>
#include <cnoid/ToolBar>
#include "gettext.h"

using namespace std;
using namespace cnoid;

namespace {

Menu* menu_Tools = nullptr;
ToolBar* fileInstance = nullptr;

}


void HamburgerMenu::initializeClass(ExtensionManager* ext)
{
    if(!menu_Tools) {
        MenuManager& mm = ext->menuManager().setPath("/" N_("Tools"));
        menu_Tools = mm.currentMenu();
    }
}


HamburgerMenu* HamburgerMenu::instance()
{
    static HamburgerMenu* instance_ = new HamburgerMenu;
    return instance_;
}


HamburgerMenu::HamburgerMenu(QWidget* parent)
    : Menu(parent)
{
    initialize();
}


HamburgerMenu::HamburgerMenu(const QString& title, QWidget* parent)
    : Menu(title, parent)
{
    initialize();
}


HamburgerMenu::~HamburgerMenu()
{

}


void HamburgerMenu::initialize()
{
    menu_ = new Menu;
    contextMenu_ = new Menu;

    auto button = fileBar()->addButton(":/GoogleMaterialSymbols/icon/menu_24dp_5F6368_FILL1_wght400_GRAD0_opsz24.svg");
    button->setMenu(menu_);
    button->sigClicked().connect([&](){ this->exec(QCursor::pos()); });

    button->setContextMenuPolicy(Qt::CustomContextMenu);
    button->connect(button, &ToolButton::customContextMenuRequested,
        [&](const QPoint& pos){ contextMenu_->exec(QCursor::pos()); });
}


namespace cnoid {

Menu* get_Tools_Menu()
{
    return menu_Tools;
}


ToolBar* fileBar()
{
    if(!fileInstance) {
        vector<ToolBar*> toolBars = MainWindow::instance()->toolBars();
        for(auto& toolBar : toolBars) {
            if(toolBar->name() == "FileBar") {
                fileInstance = toolBar;
            }
        }
    }
    return fileInstance;
}

}