/**
   \file
   \author Kenta Suzuki
*/

#ifndef CNOID_IMAGE_EFFECT_PLUGIN_IMAGE_DIALOG_H
#define CNOID_IMAGE_EFFECT_PLUGIN_IMAGE_DIALOG_H

#include <cnoid/Dialog>
#include "ImageEffect.h"

namespace cnoid {

class ImageDialogImpl;

class ImageDialog : public Dialog
{
public:
    ImageDialog();
    virtual ~ImageDialog();

    double value(const int index) const;
    void setImageEffect(ImageEffect* effect);

protected:
    virtual void onAccepted() override;
    virtual void onRejected() override;

private:
    ImageDialogImpl* impl;
    friend class ImageDialogImpl;
};

}

#endif // CNOID_IMAGE_EFFECT_PLUGIN_IMAGE_DIALOG_H
