/**
   @author Kenta Suzuki
*/

#ifndef CNOID_BOOKMARK_PLUGIN_HISTORY_MANAGER_H
#define CNOID_BOOKMARK_PLUGIN_HISTORY_MANAGER_H

#include "ArchiveListDialog.h"

namespace cnoid {

class ExtensionManager;

class HistoryManager : public ArchiveListDialog
{
public:
    static void initializeClass(ExtensionManager* ext);

    HistoryManager(QWidget* parent = nullptr);
    virtual ~HistoryManager();

protected:
    virtual void onItemDoubleClicked(const std::string& text) override;

private:
    void onProjectLoaded(int level);
    void clampActions();
};

}

#endif // CNOID_BOOKMARK_PLUGIN_HISTORY_MANAGER_H