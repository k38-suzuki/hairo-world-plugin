/**
   @author Kenta Suzuki
*/

#include "BookmarkManager.h"
#include <cnoid/Action>
#include <cnoid/AppConfig>
#include <cnoid/Buttons>
#include <cnoid/CheckBox>
#include <cnoid/ExtensionManager>
#include <cnoid/ItemManager>
#include <cnoid/Menu>
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

        const QIcon icon = QIcon(":/GoogleMaterialSymbols/icon/collections_bookmark_24dp_5F6368_FILL1_wght400_GRAD0_opsz24.svg");
        auto action = new Action;
        action->setText(_("Bookmark Manager"));
        action->setIcon(icon);
        action->setToolTip(_("Show the bookmark manager"));
        action->sigTriggered().connect([&](){ bookmarkInstance->show(); });
        HamburgerMenu::instance()->addAction(action);
    }
}


BookmarkManager* BookmarkManager::instance()
{
    return bookmarkInstance;
}


BookmarkManager::BookmarkManager()
    : ArchiveListDialog()
{
    setWindowTitle(_("Bookmark Manager"));
    setArchiveKey("bookmark_list");
    setFixedSize(800, 450);

    auto button = fileBar()->addButton(":/GoogleMaterialSymbols/icon/bookmark_add_24dp_5F6368_FILL1_wght400_GRAD0_opsz24.svg");
    button->setToolTip(_("Bookmark a current project"));
    button->sigClicked().connect([&](){ onAddButtonClicked(); });

    button->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(button, &ToolButton::customContextMenuRequested,
        [&](const QPoint& pos){ this->contextMenu()->exec(QCursor::pos()); });

    const QIcon icon = QIcon(":/GoogleMaterialSymbols/icon/file_open_24dp_5F6368_FILL1_wght400_GRAD0_opsz24.svg");
    auto button2 = new ToolButton;
    button2->setIcon(icon);
    button2->sigClicked().connect([&](){ onOpenButtonClicked(); });

    autoCheck_ = new CheckBox;
    autoCheck_->setText(_("Autoplay"));
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
    vector<string> filenames = getOpenFileNames(_("Open a project"), "cnoid");
    for(auto& filename : filenames) {
        addItem(filename.c_str());
        removeDuplicates();
    }
}