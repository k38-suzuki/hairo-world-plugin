/**
   @author Kenta Suzuki
*/

#include "BookmarkManager.h"
#include <cnoid/Action>
#include <cnoid/AppConfig>
#include <cnoid/Buttons>
#include <cnoid/ExtensionManager>
#include <cnoid/ItemManager>
#include <cnoid/Menu>
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
                    bookmarkInstance->removeDuplicates();

                    bookmarkInstance->addAction(filename);
                }
            });

        button->setContextMenuPolicy(Qt::CustomContextMenu);
        button->connect(button, &ToolButton::customContextMenuRequested,
            [&](const QPoint& pos){ bookmarkInstance->contextMenu()->exec(QCursor::pos()); });

        const QIcon icon = QIcon(":/GoogleMaterialSymbols/icon/collections_bookmark_24dp_5F6368_FILL1_wght400_GRAD0_opsz24.svg");
        auto action = new Action;
        action->setText(_("Bookmark Manager"));
        action->setIcon(icon);
        action->setToolTip(_("Show the bookmark manager"));
        action->sigTriggered().connect([&](){ bookmarkInstance->show(); });
        HamburgerMenu::instance()->subMenu()->addAction(action);
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
    autoCheck_ = new CheckBox;
    autoCheck_->setText(_("Autoplay"));
    addWidget(button);
    addWidget(autoCheck_);

    menu_ = new Menu;
    auto action = menu_->addAction(_("Clear bookmarks"));
    connect(action, &Action::triggered, [&](){ onClearActionTriggered(); });
    menu_->addSeparator();

    auto config = AppConfig::archive()->openMapping("bookmark_manager");
    if(config->isValid()) {
        autoCheck_->setChecked(config->get("auto_play", false));
    }

    auto& recentList = *AppConfig::archive()->findListing("simple_bookmark");
    if(recentList.isValid() && !recentList.empty()) {
        for(int i = 0; i < recentList.size(); ++i) {
            if(recentList[i].isString()) {
                addAction(recentList[i].toString());
            }
        }
    }
}


BookmarkManager::~BookmarkManager()
{
    auto config = AppConfig::archive()->openMapping("bookmark_manager");
    config->write("auto_play", autoCheck_->isChecked());

    QStringList list;
    auto& recentList = *AppConfig::archive()->openListing("simple_bookmark");
    recentList.clear();

    for(int i = 2; i < menu_->actions().size(); ++i) {
        recentList.append(menu_->actions().at(i)->text().toStdString(), DOUBLE_QUOTED);
    }

    if(recentList.empty()) {
        AppConfig::archive()->remove("simple_bookmark");
    }
}


void BookmarkManager::addAction(const string& filename)
{
    if(!filename.empty()) {
        for(int i = 2; i < menu_->actions().size(); ++i) {
            auto action = menu_->actions().at(i);
            if(action->text().toStdString() == filename) {
                menu_->removeAction(action);
            }
        }
    }

    auto action = menu_->addAction(filename.c_str());
    connect(action, &QAction::triggered, [this, filename](){ onLoadActionTriggered(filename); });
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
                if(autoCheck_->isChecked()) {
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
        removeDuplicates();
    }
}


void BookmarkManager::onLoadActionTriggered(const string& filename)
{
    ProjectManager* pm = ProjectManager::instance();
    bool result = pm->tryToCloseProject();
    if(result) {
        pm->clearProject();
        pm->loadProject(filename);
    }
}


void BookmarkManager::onClearActionTriggered()
{
    while(menu_->actions().size() > 2) {
        auto action = menu_->actions().at(2);
        menu_->removeAction(action);
    }
}