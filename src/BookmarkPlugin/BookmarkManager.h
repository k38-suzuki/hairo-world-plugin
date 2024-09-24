/**
   @author Kenta Suzuki
*/

#ifndef CNOID_BOOKMARK_PLUGIN_BOOKMARK_MANAGER_H
#define CNOID_BOOKMARK_PLUGIN_BOOKMARK_MANAGER_H

#include "ArchiveListDialog.h"

namespace cnoid {

class CheckBox;
class ExtensionManager;
class Menu;

class BookmarkManager : public ArchiveListDialog
{
public:
    static void initializeClass(ExtensionManager* ext);
    static BookmarkManager* instance();

    BookmarkManager();
    virtual ~BookmarkManager();

protected:
    virtual void onItemDoubleClicked(const std::string& text) override;

private:
    void addAction(const std::string& filename);
    void onAddButtonClicked();
    void onOpenButtonClicked();
    void onLoadActionTriggered(const std::string& filename);

    CheckBox* autoCheck_;
    Menu* contextMenu_;
};

}

#endif // CNOID_BOOKMARK_PLUGIN_BOOKMARK_MANAGER_H