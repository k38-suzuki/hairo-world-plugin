/**
   \file
   \author Kenta Suzuki
*/

#ifndef CNOID_BOOKMARKPLUGIN_BOOKMARKMANAGERDIALOG_H
#define CNOID_BOOKMARKPLUGIN_BOOKMARKMANAGERDIALOG_H

#include <cnoid/Dialog>
#include <cnoid/ExtensionManager>

namespace cnoid {

class BookmarkManagerDialogImpl;

class BookmarkManagerDialog : public Dialog
{
public:
    BookmarkManagerDialog();
    virtual ~BookmarkManagerDialog();

    static void initializeClass(ExtensionManager* ext);

protected:
    virtual void onAccepted() override;
    virtual void onRejected() override;

private:
    BookmarkManagerDialogImpl* impl;
    friend class BookmarkManagerDialogImpl;
};

}

#endif // CNOID_BOOKMARKPLUGIN_BOOKMARKMANAGERDIALOG_H
