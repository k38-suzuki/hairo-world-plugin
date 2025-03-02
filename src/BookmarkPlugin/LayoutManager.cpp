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
#include <cnoid/UTF8>
#include <cnoid/YAMLWriter>
#include <cnoid/stdx/filesystem>
#include "HamburgerMenu.h"
#include "gettext.h"

using namespace std;
using namespace cnoid;
namespace filesystem = cnoid::stdx::filesystem;

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
    string filename = getSaveFileName(_("CNOID File"), "cnoid");
    if(!filename.empty()) {
        filesystem::path path(fromUTF8(filename));
        string ext = path.extension().string();
        if(ext != ".cnoid"){
            filename += ".cnoid";
        }

        YAMLWriter yamlWriter;
        yamlWriter.setKeyOrderPreservationMode(true);
        if(yamlWriter.openFile(filename)) {
            auto layout = ProjectManager::instance()->storeCurrentLayout();
            yamlWriter.putNode(layout);
            yamlWriter.closeFile();
        }
        addItem(filename.c_str());
        removeDuplicates();
    }
}


void LayoutManager::onOpenButtonClicked()
{
    vector<string> filenames = getOpenFileNames(_("CNOID File"), "cnoid");
    for(auto& filename : filenames) {
        addItem(filename.c_str());
        removeDuplicates();
    }
}