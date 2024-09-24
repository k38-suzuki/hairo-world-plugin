/**
   @author Kenta Suzuki
*/

#ifndef CNOID_BOOKMARK_PLUGIN_HISTORY_MANAGER_H
#define CNOID_BOOKMARK_PLUGIN_HISTORY_MANAGER_H

#include "ArchiveListDialog.h"

namespace cnoid {

class ExtensionManager;
class Menu;

class HistoryManager : public ArchiveListDialog
{
public:
    static void initializeClass(ExtensionManager* ext);

    HistoryManager();
    virtual ~HistoryManager();

protected:
    virtual void onItemDoubleClicked(const std::string& text) override;

private:
    void addProject(const std::string& filename);
    void onProjectLoaded(int level);
    void onLoadActionTriggered(const std::string& filename);

    Menu* contextMenu_;
};

}

#endif // CNOID_BOOKMARK_PLUGIN_HISTORY_MANAGER_H