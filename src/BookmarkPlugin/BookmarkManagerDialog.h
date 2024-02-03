/**
   @author Kenta Suzuki
*/

#ifndef CNOID_BOOKMARK_PLUGIN_BOOKMARK_MANAGER_DIALOG_H
#define CNOID_BOOKMARK_PLUGIN_BOOKMARK_MANAGER_DIALOG_H

#include <cnoid/Dialog>

namespace cnoid {

class BookmarkManagerDialog : public Dialog
{
public:
    BookmarkManagerDialog();
    virtual ~BookmarkManagerDialog();

    static BookmarkManagerDialog* instance();

    void addProjectFile(const std::string& filename);

private:
    class Impl;
    Impl* impl;
};

}

#endif
