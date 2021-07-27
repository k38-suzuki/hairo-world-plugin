/**
   \file
   \author Kenta Suzuki
*/

#ifndef CNOID_OMPLPLUGIN_MOTIONPLANNERDIALOG_H
#define CNOID_OMPLPLUGIN_MOTIONPLANNERDIALOG_H

#include <cnoid/Dialog>
#include <cnoid/ExtensionManager>

namespace cnoid {

class MotionPlannerDialogImpl;

class MotionPlannerDialog : public Dialog
{
public:
    MotionPlannerDialog();
    virtual ~MotionPlannerDialog();

    static void initializeClass(ExtensionManager* ext);
    static MotionPlannerDialog* instance();

protected:
    virtual void onAccepted();
    virtual void onRejected();

private:
    MotionPlannerDialogImpl* impl;
    friend class MotionPlannerDialogImpl;
};

}

#endif // CNOID_OMPLPLUGIN_MOTIONPLANNERDIALOG_H
