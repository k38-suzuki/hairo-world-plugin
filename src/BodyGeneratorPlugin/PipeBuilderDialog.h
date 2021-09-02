/**
   \file
   \author Kenta Suzuki
*/

#ifndef CNOID_BODYGENERATORPLUGIN_PIPEBUILDERDIALOG_H
#define CNOID_BODYGENERATORPLUGIN_PIPEBUILDERDIALOG_H

#include <cnoid/Dialog>

namespace cnoid {

class PipeBuilderDialogImpl;

class PipeBuilderDialog : public Dialog
{
public:
    PipeBuilderDialog();
    virtual ~PipeBuilderDialog();

    static PipeBuilderDialog* instance();

    bool save(const std::string& filename);

protected:
    virtual void onAccepted();
    virtual void onRejected();

private:
    PipeBuilderDialogImpl* impl;
    friend class PipeBuilderDialogImpl;
};

}

#endif // CNOID_BODYGENERATORPLUGIN_PIPEBUILDERDIALOG_H
