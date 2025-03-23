/**
   @author Kenta Suzuki
*/

#include "BookmarkManager.h"
#include <cnoid/AppConfig>
#include <cnoid/Buttons>
#include <cnoid/CheckBox>
#include <cnoid/ExtensionManager>
#include <cnoid/ItemManager>
#include <cnoid/ProjectManager>
#include <cnoid/SimulationBar>
#include <cnoid/ValueTree>
#include "HamburgerMenu.h"
#include "gettext.h"

using namespace std;
using namespace cnoid;

namespace {

BookmarkManager* bookmarkInstance = nullptr;

}


void BookmarkManager::initializeClass(ExtensionManager* ext)
{
    if(!bookmarkInstance) {
        bookmarkInstance = ext->manage(new BookmarkManager);
    }
}


BookmarkManager* BookmarkManager::instance()
{
    return bookmarkInstance;
}


BookmarkManager::BookmarkManager(QWidget* parent)
    : ArchiveListWidget(parent)
{
    setWindowTitle(_("Bookmark Manager"));
    setArchiveKey("bookmark_list");
    setMinimumSize(640, 480);

    auto button1 = new ToolButton(_("New"));
    button1->sigClicked().connect([&](){ onAddButtonClicked(); });

    button1->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(button1, &ToolButton::customContextMenuRequested,
        [&](const QPoint& pos){ this->contextMenu()->exec(QCursor::pos()); });

    auto button2 = new ToolButton(_("Open"));
    button2->sigClicked().connect([&](){ onOpenButtonClicked(); });

    autoCheck_ = new CheckBox;
    autoCheck_->setText(_("Autoplay"));

    addWidget(button1);
    addWidget(button2);
    addWidget(autoCheck_);

    auto config = AppConfig::archive()->openMapping("bookmark_manager");
    if(config->isValid()) {
        autoCheck_->setChecked(config->get("auto_play", false));
    }
}


BookmarkManager::~BookmarkManager()
{
    auto config = AppConfig::archive()->openMapping("bookmark_manager");
    config->write("auto_play", autoCheck_->isChecked());
}


void BookmarkManager::onItemDoubleClicked(const string& text)
{
    if(loadProject(text)) {
        if(autoCheck_->isChecked()) {
            SimulationBar::instance()->startSimulation(true);
        }
    }
}


void BookmarkManager::onAddButtonClicked()
{
    string filename = ProjectManager::instance()->currentProjectFile();
    if(!filename.empty()) {
        addItem(filename.c_str());
        removeDuplicates();
    }
}


void BookmarkManager::onOpenButtonClicked()
{
    vector<string> filenames = getOpenFileNames(_("CNOID File"), "cnoid");
    for(auto& filename : filenames) {
        addItem(filename.c_str());
        removeDuplicates();
    }
}