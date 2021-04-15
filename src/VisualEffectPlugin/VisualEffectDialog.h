/**
   \file
   \author Kenta Suzuki
*/

#ifndef CNOID_VISUAL_EFFECT_PLUGIN_IMAGE_DIALOG_H
#define CNOID_VISUAL_EFFECT_PLUGIN_IMAGE_DIALOG_H

#include <cnoid/Dialog>
#include <cnoid/ExtensionManager>
#include "VisualEffect.h"

namespace cnoid {

class VisualEffectDialogImpl;

class VisualEffectDialog : public Dialog
{
public:
    VisualEffectDialog();
    virtual ~VisualEffectDialog();

    static void initializeClass(ExtensionManager* ext);
    static VisualEffectDialog* instance();

    void setVisualEffect(const VisualEffect& effect);

    void setHue(const double& hue);
    double hue() const;
    void setSaturation(const double& saturation);
    double saturation() const;
    void setValue(const double& value);
    double value() const;
    void setRed(const double& red);
    double red() const;
    void setGreen(const double& green);
    double green() const;
    void setBlue(const double& blue);
    double blue() const;
    void setCoefB(const double& coefB);
    double coefB() const;
    void setCoefD(const double& coefD);
    double coefD() const;
    void setStdDev(const double& stdDev);
    double stdDev() const;
    void setSalt(const double& salt);
    double salt() const;
    void setPepper(const double& pepper);
    double pepper() const;
    void setFlip(const double& flip);
    bool flip() const;
    void setFilter(const int& filter);
    int filter() const;

protected:
    virtual void onAccepted() override;
    virtual void onRejected() override;

private:
    VisualEffectDialogImpl* impl;
    friend class VisualEffectDialogImpl;
};

}

#endif // CNOID_VISUAL_EFFECT_PLUGIN_IMAGE_DIALOG_H
