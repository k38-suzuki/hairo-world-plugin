/**
   \file
   \author Kenta Suzuki
*/

#ifndef CNOID_INERTIA_CALCULATOR_PLUGIN_INERTIA_CALCULATOR_DIALOG_H
#define CNOID_INERTIA_CALCULATOR_PLUGIN_INERTIA_CALCULATOR_DIALOG_H

#include <cnoid/Dialog>
#include <cnoid/ExtensionManager>

namespace cnoid {

class InertiaCalculatorDialogImpl;

class InertiaCalculatorDialog : public Dialog
{
public:
    InertiaCalculatorDialog();
    virtual ~InertiaCalculatorDialog();

    static void initializeClass(ExtensionManager* ext);

protected:
    virtual void onAccepted() override;
    virtual void onRejected() override;

private:
    InertiaCalculatorDialogImpl* impl;
    friend class InertiaCalculatorDialogImpl;
};

}

#endif // CNOID_INERTIA_CALCULATOR_PLUGIN_INERTIA_CALCULATOR_DIALOG_H
