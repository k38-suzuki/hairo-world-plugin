/**
   \file
   \author Kenta Suzuki
*/

#include <cnoid/MenuManager>
#include <cnoid/Plugin>
#include <cnoid/ProjectManager>
#include <fmt/format.h>
#include "BookmarkBar.h"
#include "BookmarkManagerView.h"
#include "HistoryManager.h"
#include "gettext.h"

using namespace cnoid;

namespace {

HistoryManager* hm = nullptr;
ProjectManager* pm = nullptr;

class BookmarkPlugin : public Plugin
{
public:

    BookmarkPlugin() : Plugin("Bookmark")
    {

    }

    virtual bool initialize() override
    {
        BookmarkBar::initializeClass(this);
//        BookmarkManagerView::initializeClass(this);
        HistoryManager::initializeClass(this);

        hm = HistoryManager::instance();
        pm = ProjectManager::instance();

        std::vector<std::string> histories = hm->histories();
        for(int i = 0; i < histories.size(); ++i) {
            onProjectLoaded(histories[i]);
        }

        pm->sigProjectLoaded().connect([&](int level){ onProjectLoaded(pm->currentProjectFile()); });

        return true;
    }

    virtual const char* description() const override
    {
        static std::string text =
            fmt::format("Bookmark Plugin Version {}\n", CNOID_FULL_VERSION_STRING) +
            "\n" +
            "Copyright (c) 2021 Japan Atomic Energy Agency.\n"
            "\n" +
            MITLicenseText();
        return text.c_str();
    }

private:

    void onProjectLoaded(const std::string projectFile)
    {
        MenuManager& mm = menuManager().setPath("/Tools").setPath(_("History"));
        if(!projectFile.empty()) {
            Action* historyItem = mm.addItem(projectFile);
            hm->addHistory(projectFile);

            QWidget* current = mm.current();
            QMenu* menu = dynamic_cast<QMenu*>(current);
            if(menu) {
                int size = menu->actions().size();
                int max = hm->maxHistory();
                if(size > max) {
                    Action* action = dynamic_cast<Action*>(menu->actions()[0]);
                    menu->removeAction(action);
                }
            }

            historyItem->sigTriggered().connect([&, historyItem]() {
                std::string filename = historyItem->text().toStdString();
                bool on = BookmarkManagerView::openDialogToLoadProject(filename);
                if(!on) {
                    return;
                }
            });
        }
    }
};

}

CNOID_IMPLEMENT_PLUGIN_ENTRY(BookmarkPlugin)
