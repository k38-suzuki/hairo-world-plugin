/**
   \file
   \author Kenta Suzuki
*/

#ifndef CNOID_VISUAL_EFFECT_PLUGIN_IMAGE_DIALOG_H
#define CNOID_VISUAL_EFFECT_PLUGIN_IMAGE_DIALOG_H

#include <cnoid/Dialog>
#include "VisualEffect.h"

namespace cnoid {

class VisualEffectDialogImpl;

class VisualEffectDialog : public Dialog
{
public:
    VisualEffectDialog();
    virtual ~VisualEffectDialog();

    double value(const int index) const;
    void setVisualEffect(VisualEffect* effect);

protected:
    virtual void onAccepted() override;
    virtual void onRejected() override;

private:
    VisualEffectDialogImpl* impl;
    friend class VisualEffectDialogImpl;
};

}

#endif // CNOID_VISUAL_EFFECT_PLUGIN_IMAGE_DIALOG_H
