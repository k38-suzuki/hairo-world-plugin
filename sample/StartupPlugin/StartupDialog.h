/**
   \file
   \author Kenta Suzuki
*/

#ifndef CNOID_STARTUPPLUGIN_STARTUPDIALOG_H
#define CNOID_STARTUPPLUGIN_STARTUPDIALOG_H

#include <cnoid/Dialog>
#include <cnoid/ExtensionManager>

namespace cnoid {

class StartupDialogImpl;

class StartupDialog : public Dialog
{
public:
    StartupDialog();
    virtual ~StartupDialog();

    static void initializeClass(ExtensionManager* ext);

protected:
    virtual void onAccepted() override;
    virtual void onRejected() override;

private:
    StartupDialogImpl* impl;
    friend class StartupDialogImpl;
};

}

#endif // CNOID_STARTUPPLUGIN_STARTUPDIALOG_H
