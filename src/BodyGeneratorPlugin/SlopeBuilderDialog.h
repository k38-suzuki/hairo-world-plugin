/**
   \file
   \author Kenta Suzuki
*/

#ifndef CNOID_BODYGENERATORPLUGIN_SLOPEBUILDERDIALOG_H
#define CNOID_BODYGENERATORPLUGIN_SLOPEBUILDERDIALOG_H

#include <cnoid/Dialog>

namespace cnoid {

class SlopeBuilderDialogImpl;

class SlopeBuilderDialog : public Dialog
{
public:
    SlopeBuilderDialog();
    virtual ~SlopeBuilderDialog();

    static SlopeBuilderDialog* instance();

    bool save(const std::string& filename);

protected:
    virtual void onAccepted();
    virtual void onRejected();

private:
    SlopeBuilderDialogImpl* impl;
    friend class SlopeBuilderDialogImpl;
};

}

#endif // CNOID_BODYGENERATORPLUGIN_SLOPEBUILDERDIALOG_H
