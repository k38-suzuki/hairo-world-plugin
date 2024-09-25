/**
    @author Kenta Suzuki
*/

#include "LayoutManager.h"
#include <cnoid/Action>
#include <cnoid/Buttons>
#include <cnoid/ExtensionManager>
#include <cnoid/ItemManager>
#include <cnoid/ProjectManager>
#include <cnoid/RootItem>
#include <cnoid/ToolBar>
#include <QFileDialog>
#include "HamburgerMenu.h"
#include "gettext.h"

using namespace std;
using namespace cnoid;

namespace {

LayoutManager* layoutInstance = nullptr;

}


void LayoutManager::initializeClass(ExtensionManager* ext)
{
    if(!layoutInstance) {
        layoutInstance = ext->manage(new LayoutManager);

        const QIcon icon = QIcon(":/GoogleMaterialSymbols/icon/dashboard_24dp_5F6368_FILL1_wght400_GRAD0_opsz24.svg");
        auto action = new Action;
        action->setText(_("Layout Manager"));
        action->setIcon(icon);
        action->setToolTip(_("Show the layout manager"));
        action->sigTriggered().connect([&](){ layoutInstance->show(); });
        HamburgerMenu::instance()->addAction(action);
    }
}


LayoutManager::LayoutManager(QWidget* parent)
    : ArchiveListDialog(parent)
{
    setWindowTitle(_("Layout Manager"));
    setArchiveKey("layout_list");
    setFixedSize(800, 450);

    auto button1 = fileBar()->addButton(":/GoogleMaterialSymbols/icon/dashboard_customize_24dp_5F6368_FILL1_wght400_GRAD0_opsz24.svg");
    button1->setToolTip(_("Save a current layout"));
    button1->sigClicked().connect([&](){ onSaveButtonClicked(); });

    button1->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(button1, &ToolButton::customContextMenuRequested,
        [&](const QPoint& pos){ this->contextMenu()->exec(QCursor::pos()); });

    const QIcon icon = QIcon(":/GoogleMaterialSymbols/icon/file_open_24dp_5F6368_FILL1_wght400_GRAD0_opsz24.svg");
    auto button2 = new ToolButton;
    button2->setIcon(icon);
    button2->sigClicked().connect([&](){ onOpenButtonClicked(); });

    addWidget(button2);
}


LayoutManager::~LayoutManager()
{

}


void LayoutManager::onItemDoubleClicked(const string& text)
{
    ProjectManager::instance()->loadProject(text, RootItem::instance());
}


void LayoutManager::onSaveButtonClicked()
{
    string filename = getSaveFileName(_("Save a project"), "cnoid");
    if(!filename.empty()) {
        ProjectManager::instance()->saveProject(filename);
        addItem(filename.c_str());
        removeDuplicates();
    }
}


void LayoutManager::onOpenButtonClicked()
{
    vector<string> filenames = getOpenFileNames(_("Open a project"), "cnoid");
    for(auto& filename : filenames) {
        addItem(filename.c_str());
        removeDuplicates();
    }
}