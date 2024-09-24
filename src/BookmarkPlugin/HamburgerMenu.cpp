/**
    @author Kenta Suzuki
*/

#include "HamburgerMenu.h"
#include <cnoid/Action>
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


void HamburgerMenu::setClearableContext(const string& text)
{
    if(!text.empty()) {
        auto action = contextMenu_->addAction(text.c_str());
        connect(action, &QAction::triggered, [&](){ onClearActionTriggered(); });
        contextMenu_->addSeparator();
    }
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


void HamburgerMenu::onClearActionTriggered()
{
    while(contextMenu_->actions().size() > 2) {
        auto action = contextMenu_->actions().at(2);
        contextMenu_->removeAction(action);
    }
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