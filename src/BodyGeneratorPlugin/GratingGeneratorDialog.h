/**
   \file
   \author Kenta Suzuki
*/

#ifndef CNOID_BODY_GENERATOR_PLUGIN_GRATING_GENERATOR_DIALOG_H
#define CNOID_BODY_GENERATOR_PLUGIN_GRATING_GENERATOR_DIALOG_H

#include <cnoid/Dialog>

namespace cnoid {

class GratingGeneratorDialogImpl;

class GratingGeneratorDialog : public Dialog
{
public:
    GratingGeneratorDialog();
    virtual ~GratingGeneratorDialog();

    static GratingGeneratorDialog* instance();


private:
    GratingGeneratorDialogImpl* impl;
    friend class GratingGeneratorDialogImpl;
};

}

#endif // CNOID_BODY_GENERATOR_PLUGIN_GRATING_GENERATOR_DIALOG_H