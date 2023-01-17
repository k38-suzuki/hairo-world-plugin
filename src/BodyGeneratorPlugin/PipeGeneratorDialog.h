/**
   \file
   \author Kenta Suzuki
*/

#ifndef CNOID_BODY_GENERATOR_PLUGIN_PIPE_GENERATOR_DIALOG_H
#define CNOID_BODY_GENERATOR_PLUGIN_PIPE_GENERATOR_DIALOG_H

#include <cnoid/Dialog>

namespace cnoid {

class PipeGeneratorDialogImpl;

class PipeGeneratorDialog : public Dialog
{
public:
    PipeGeneratorDialog();
    virtual ~PipeGeneratorDialog();

    static PipeGeneratorDialog* instance();

private:
    PipeGeneratorDialogImpl* impl;
    friend class PipeGeneratorDialogImpl;
};

}

#endif // CNOID_BODY_GENERATOR_PLUGIN_PIPE_GENERATOR_DIALOG_H