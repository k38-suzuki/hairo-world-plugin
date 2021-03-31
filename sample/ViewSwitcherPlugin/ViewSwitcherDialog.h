/**
   \file
   \author Kenta Suzuki
*/

#ifndef CNOID_VIEWSWITCHERPLUGIN_VIEWSWITCHERBAR_H
#define CNOID_VIEWSWITCHERPLUGIN_VIEWSWITCHERBAR_H

#include <cnoid/Dialog>
#include <cnoid/ExtensionManager>

namespace cnoid {

class ViewSwitcherDialogImpl;

class ViewSwitcherDialog : public Dialog
{
public:
    ViewSwitcherDialog();
    virtual ~ViewSwitcherDialog();

    static void initializeClass(ExtensionManager* ext);
    static void finalizeClass();

    void onButtonPressed(const int& id, const bool& isPressed);
    void onAxisPositioned(const int& id, const double& position);

protected:
    virtual void onAccepted() override;
    virtual void onRejected() override;

private:
    ViewSwitcherDialogImpl* impl;
    friend class ViewSwitcherDialogImpl;
};

}

#endif // CNOID_VIEWSWITCHERPLUGIN_VIEWSWITCHERBAR_H
