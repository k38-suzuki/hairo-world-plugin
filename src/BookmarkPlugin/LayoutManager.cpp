/**
    @author Kenta Suzuki
*/

#include "LayoutManager.h"
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
#include "ProjectListedDialog.h"
#include "ListedWidget.h"
#include "gettext.h"

using namespace std;
using namespace cnoid;
namespace filesystem = cnoid::stdx::filesystem;


void LayoutManager::initializeClass(ExtensionManager* ext)
{
    static LayoutManager* widget = nullptr;

    if(!widget) {
        widget = ext->manage(new LayoutManager);

        ProjectListedDialog::instance()->listedWidget()->addWidget(_("Layout"), widget);
    }
}


LayoutManager::LayoutManager(QWidget* parent)
    : ArchiveListWidget(parent)
{
    setWindowTitle(_("Layout Manager"));
    setArchiveKey("layout_list");
    setMinimumSize(640, 480);

    auto button1 = new ToolButton(_("New"));
    button1->sigClicked().connect([&](){ onSaveButtonClicked(); });

    button1->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(button1, &ToolButton::customContextMenuRequested,
        [&](const QPoint& pos){ this->contextMenu()->exec(QCursor::pos()); });

    auto button2 = new ToolButton(_("Open"));
    button2->sigClicked().connect([&](){ onOpenButtonClicked(); });

    addWidget(button1);
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