/**
   @author Kenta Suzuki
*/

#ifndef CNOID_BOOKMARK_PLUGIN_BOOKMARK_MANAGER_H
#define CNOID_BOOKMARK_PLUGIN_BOOKMARK_MANAGER_H

#include "ArchiveListDialog.h"
#include <cnoid/CheckBox>

namespace cnoid {

class ExtensionManager;

class BookmarkManager : public ArchiveListDialog
{
public:
    static void initializeClass(ExtensionManager* ext);
    static BookmarkManager* instance();

    BookmarkManager();
    virtual ~BookmarkManager();

protected:
    virtual void onItemDoubleClicked(std::string& text) override;

private:
    void onButtonClicked();

    CheckBox* autoCheck;
};

}

#endif // CNOID_BOOKMARK_PLUGIN_BOOKMARK_MANAGER_H
