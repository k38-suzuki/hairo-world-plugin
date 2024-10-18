/**
   @author Kenta Suzuki
*/

#ifndef CNOID_BOOKMARK_PLUGIN_BOOKMARK_MANAGER_H
#define CNOID_BOOKMARK_PLUGIN_BOOKMARK_MANAGER_H

#include "ArchiveListWidget.h"

namespace cnoid {

class CheckBox;
class ExtensionManager;

class BookmarkManager : public ArchiveListWidget
{
public:
    static void initializeClass(ExtensionManager* ext);
    static BookmarkManager* instance();

    BookmarkManager(QWidget* parent = nullptr);
    virtual ~BookmarkManager();

protected:
    virtual void onItemDoubleClicked(const std::string& text) override;

private:
    void onAddButtonClicked();
    void onOpenButtonClicked();

    CheckBox* autoCheck_;
};

}

#endif // CNOID_BOOKMARK_PLUGIN_BOOKMARK_MANAGER_H