/**
   \file
   \author Kenta Suzuki
*/

#ifndef CNOID_INERTIACALCULATORPLUGIN_INERTIACALCULATORDIALOG_H
#define CNOID_INERTIACALCULATORPLUGIN_INERTIACALCULATORDIALOG_H

#include <cnoid/Dialog>
#include <cnoid/ExtensionManager>

namespace cnoid {

class InertiaCalculatorDialogImpl;

class InertiaCalculatorDialog : public Dialog
{
public:
    InertiaCalculatorDialog();
    virtual ~InertiaCalculatorDialog();

    static void initialize(ExtensionManager* ext);

protected:
    virtual void onAccepted() override;
    virtual void onRejected() override;

private:
    InertiaCalculatorDialogImpl* impl;
    friend class InertiaCalculatorDialogImpl;
};

}

#endif // CNOID_INERTIACALCULATORPLUGIN_INERTIACALCULATORDIALOG_H
