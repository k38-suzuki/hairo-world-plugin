/**
   @author Kenta Suzuki
*/

#include "BookmarkManager.h"
#include <cnoid/Archive>
#include <cnoid/AppConfig>
#include <cnoid/Buttons>
#include <cnoid/ExtensionManager>
#include <cnoid/ItemManager>
#include <cnoid/ProjectManager>
#include <cnoid/SimulationBar>
#include <cnoid/stdx/filesystem>
#include <cnoid/ToolsUtil>
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

        auto button1 = fileBar()->addButton(QIcon::fromTheme("list-add"));
        button1->setToolTip(_("Bookmark a current project"));
        button1->sigClicked().connect(
            [&](){
                const string& filename = ProjectManager::instance()->currentProjectFile();
                if(!filename.empty()) {
                    bookmarkInstance->addItem(filename.c_str());
                }                
            });

        auto button2 = fileBar()->addButton(QIcon::fromTheme("user-bookmarks"));
        button2->setToolTip(_("Show the bookmark manager"));
        button2->sigClicked().connect([&](){ bookmarkInstance->show(); });
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

    auto button = new PushButton;
    button->setIcon(QIcon::fromTheme("document-open"));
    button->sigClicked().connect([&](){ onButtonClicked(); });
    autoCheck = new CheckBox;
    autoCheck->setText(_("Autoplay"));
    addWidget(button);
    addWidget(autoCheck);

    auto& archive = *AppConfig::archive()->openMapping("bookmark_manager");
    if(archive.isValid()) {
        autoCheck->setChecked(archive.get("auto_play", false));
    }
}


BookmarkManager::~BookmarkManager()
{
    auto& archive = *AppConfig::archive()->openMapping("bookmark_manager");
    archive.write("auto_play", autoCheck->isChecked());
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


void BookmarkManager::onButtonClicked()
{
    vector<string> filenames = getOpenFileNames(_("Open a project"), "cnoid");
    for(auto& filename : filenames) {
        addItem(filename.c_str());
    }
}