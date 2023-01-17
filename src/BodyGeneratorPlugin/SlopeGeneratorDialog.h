/**
   \file
   \author Kenta Suzuki
*/

#ifndef CNOID_BODY_GENERATOR_PLUGIN_SLOPE_GENERATOR_DIALOG_H
#define CNOID_BODY_GENERATOR_PLUGIN_SLOPE_GENERATOR_DIALOG_H

#include <cnoid/Dialog>

namespace cnoid {

class SlopeGeneratorDialogImpl;

class SlopeGeneratorDialog : public Dialog
{
public:
    SlopeGeneratorDialog();
    virtual ~SlopeGeneratorDialog();

    static SlopeGeneratorDialog* instance();

private:
    SlopeGeneratorDialogImpl* impl;
    friend class SlopeGeneratorDialogImpl;
};

}

#endif // CNOID_BODY_GENERATOR_PLUGIN_SLOPE_GENERATOR_DIALOG_H