/**
   @author Kenta Suzuki
*/

#include "HistoryManager.h"
#include <cnoid/AppConfig>
#include <cnoid/ExtensionManager>
#include <cnoid/Menu>
#include <cnoid/MenuManager>
#include <cnoid/MessageView>
#include <cnoid/ProjectManager>
#include <cnoid/ValueTree>
#include "ToolsUtil.h"
#include "gettext.h"

using namespace std;
using namespace cnoid;

namespace {

HistoryManager* historyInstance = nullptr;
Menu* currentMenu = nullptr;

}

namespace cnoid {

class HistoryManager::Impl
{
public:
    HistoryManager* self;

    Impl(HistoryManager* self);
    ~Impl();

    void loadProject(const string& filename);
    void addProject(const string& filename);
    void clearProjects();
    void onProjectLoaded(int level);
};

}


void HistoryManager::initializeClass(ExtensionManager* ext)
{
    MenuManager& mm = ext->menuManager().setPath("/" N_("Tools")).setPath(_("History"));
    currentMenu = mm.currentMenu();

    if(!historyInstance) {
        historyInstance = ext->manage(new HistoryManager);

        auto button = fileBar()->addButton(":/GoogleMaterialSymbols/icon/history_24dp_5F6368.svg");
        button->setToolTip(_("Show the history manager"));
        button->sigClicked().connect([&](){ historyInstance->show(); });
    }
}


HistoryManager::HistoryManager()
    : ArchiveListDialog()
{
    impl = new Impl(this);
}


HistoryManager::Impl::Impl(HistoryManager* self)
    : self(self)
{
    QAction* action = currentMenu->addAction(_("Clear histories"));
    self->connect(action, &QAction::triggered, [&](){ clearProjects(); });
    currentMenu->addSeparator();

    auto& recentFiles = *AppConfig::archive()->findListing("histories");
    if(recentFiles.isValid() && !recentFiles.empty()) {
        for(int i = 0; i < recentFiles.size(); ++i) {
            if(recentFiles[i].isString()) {
                addProject(recentFiles[i].toString());
            }
        }
    }

    self->setWindowTitle(_("History Manager"));
    self->setArchiveKey("history_list");
    self->setFixedSize(800, 450);

    ProjectManager::instance()->sigProjectLoaded().connect([&](int level){ onProjectLoaded(level); });
}


HistoryManager::~HistoryManager()
{
    delete impl;
}


HistoryManager::Impl::~Impl()
{
    auto& recentFiles = *AppConfig::archive()->openListing("histories");
    recentFiles.clear();

    for(int i = 2; i < currentMenu->actions().size(); ++i) {
        string filename = currentMenu->actions().at(i)->text().toStdString();
        recentFiles.append(filename, DOUBLE_QUOTED);
    }

    if(recentFiles.empty()) {
        AppConfig::archive()->remove("histories");
    }
}


void HistoryManager::onItemDoubleClicked(const string& text)
{
    impl->loadProject(text);
}


void HistoryManager::Impl::loadProject(const string& filename)
{
    if(!filename.empty()) {
        if(ProjectManager::instance()->tryToCloseProject()) {
            ProjectManager::instance()->clearProject();
            MessageView::instance()->flush();
            ProjectManager::instance()->loadProject(filename);
        }
    }
}


void HistoryManager::Impl::addProject(const string& filename)
{
    if(!filename.empty()) {
        for(int i = 2; i < currentMenu->actions().size(); ++i) {
            QAction* action = currentMenu->actions().at(i);
            if(action->text().toStdString() == filename) {
                currentMenu->removeAction(action);
            }
        }

        if(currentMenu->actions().size() >= 12) {
            QAction* action = currentMenu->actions().at(2);
            currentMenu->removeAction(action);
        }

        QAction* action = currentMenu->addAction(filename.c_str());
        self->connect(action, &QAction::triggered, [this, filename](){ loadProject(filename); });
    }
}


void HistoryManager::Impl::clearProjects()
{
    while(currentMenu->actions().size() > 2) {
        QAction* action = currentMenu->actions().at(2);
        currentMenu->removeAction(action);
    }
}


void HistoryManager::Impl::onProjectLoaded(int level)
{
    string filename = ProjectManager::instance()->currentProjectFile().c_str();
    if(!filename.empty()) {
        addProject(filename);
        self->addItem(filename.c_str());
        self->removeDuplicates();
    }
}