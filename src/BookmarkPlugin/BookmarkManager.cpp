/**
   @author Kenta Suzuki
*/

#include "BookmarkManager.h"
#include <cnoid/Action>
#include <cnoid/AppConfig>
#include <cnoid/Buttons>
#include <cnoid/ExtensionManager>
#include <cnoid/ItemManager>
#include <cnoid/ProjectManager>
#include <cnoid/SimulationBar>
#include <cnoid/ValueTree>
#include <cnoid/stdx/filesystem>
#include "HamburgerMenu.h"
#include "gettext.h"

using namespace std;
using namespace cnoid;
namespace filesystem = cnoid::stdx::filesystem;

namespace {

BookmarkManager* bookmarkInstance = nullptr;

}


void BookmarkManager::initializeClass(ExtensionManager* ext)
{
    if(!bookmarkInstance) {
        bookmarkInstance = ext->manage(new BookmarkManager);

        auto button = fileBar()->addButton(":/GoogleMaterialSymbols/icon/bookmark_add_24dp_5F6368_FILL1_wght400_GRAD0_opsz24.svg");
        button->setToolTip(_("Bookmark a current project"));
        button->sigClicked().connect(
            [&](){
                const string& filename = ProjectManager::instance()->currentProjectFile();
                if(!filename.empty()) {
                    bookmarkInstance->addItem(filename.c_str());
                }
            });

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

    const QIcon icon = QIcon(":/GoogleMaterialSymbols/icon/file_open_24dp_5F6368_FILL1_wght400_GRAD0_opsz24.svg");
    auto button = new ToolButton;
    button->setIcon(icon);
    button->sigClicked().connect([&](){ onOpenButtonClicked(); });
    autoCheck = new CheckBox;
    autoCheck->setText(_("Autoplay"));
    addWidget(button);
    addWidget(autoCheck);

    auto config = AppConfig::archive()->openMapping("bookmark_manager");
    if(config->isValid()) {
        autoCheck->setChecked(config->get("auto_play", false));
    }
}


BookmarkManager::~BookmarkManager()
{
    auto config = AppConfig::archive()->openMapping("bookmark_manager");
    config->write("auto_play", autoCheck->isChecked());
}


void BookmarkManager::onItemDoubleClicked(const string& text)
{
    if(!text.empty()) {
        filesystem::path path(text);
        string extension = path.extension().string();
        if(extension == ".cnoid") {
            ProjectManager* pm = ProjectManager::instance();
            bool result = pm->tryToCloseProject();
            if(result) {
                pm->clearProject();
                pm->loadProject(text);
                if(autoCheck->isChecked()) {
                    SimulationBar::instance()->startSimulation(true);
                }
            }
        }
    }
}


void BookmarkManager::onOpenButtonClicked()
{
    vector<string> filenames = getOpenFileNames(_("Open a project"), "cnoid");
    for(auto& filename : filenames) {
        addItem(filename.c_str());
    }
}