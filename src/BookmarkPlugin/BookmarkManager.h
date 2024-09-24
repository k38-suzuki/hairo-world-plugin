/**
   @author Kenta Suzuki
*/

#ifndef CNOID_BOOKMARK_PLUGIN_BOOKMARK_MANAGER_H
#define CNOID_BOOKMARK_PLUGIN_BOOKMARK_MANAGER_H

#include "ArchiveListDialog.h"
#include <cnoid/CheckBox>

namespace cnoid {

class Action;
class ExtensionManager;
class Menu;

class BookmarkManager : public ArchiveListDialog
{
public:
    static void initializeClass(ExtensionManager* ext);
    static BookmarkManager* instance();

    BookmarkManager();
    virtual ~BookmarkManager();

    Menu* contextMenu() { return menu_; }
    void addAction(const std::string& filename);

protected:
    virtual void onItemDoubleClicked(const std::string& text) override;

private:
    void onOpenButtonClicked();
    void onLoadActionTriggered(const std::string& filename);
    void onClearActionTriggered();

    CheckBox* autoCheck_;
    Menu* menu_;
};

}

#endif // CNOID_BOOKMARK_PLUGIN_BOOKMARK_MANAGER_H