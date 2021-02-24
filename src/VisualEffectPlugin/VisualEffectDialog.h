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

    void setVisualEffect(VisualEffect* effect);
    double hue() const;
    double saturation() const;
    double value() const;
    double red() const;
    double green() const;
    double blue() const;
    double coefB() const;
    double coefD() const;
    double stdDev() const;
    double salt() const;
    double pepper() const;
    bool flip() const;

protected:
    virtual void onAccepted() override;
    virtual void onRejected() override;

private:
    VisualEffectDialogImpl* impl;
    friend class VisualEffectDialogImpl;
};

}

#endif // CNOID_VISUAL_EFFECT_PLUGIN_IMAGE_DIALOG_H
