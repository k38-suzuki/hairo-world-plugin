/**
   \file
   \author Kenta Suzuki
*/

#ifndef CNOID_BODYGENERATORPLUGIN_GRATINGBUILDERDIALOG_H
#define CNOID_BODYGENERATORPLUGIN_GRATINGBUILDERDIALOG_H

#include <cnoid/Dialog>

namespace cnoid {

class GratingBuilderDialogImpl;

class GratingBuilderDialog : public Dialog
{
public:
    GratingBuilderDialog();
    virtual ~GratingBuilderDialog();

protected:
    virtual void onAccepted();
    virtual void onRejected();

private:
    GratingBuilderDialogImpl* impl;
    friend class GratingBuilderDialogImpl;
};

}

#endif // CNOID_BODYGENERATORPLUGIN_GRATINGBUILDERDIALOG_H
