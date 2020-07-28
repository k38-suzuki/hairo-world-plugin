/**
   \file
   \author Kenta Suzuki
*/

#ifndef CNOID_STEP_FIELD_BUILDER_PLUGIN_STEP_FIELD_BUILDER_DIALOG_H
#define CNOID_STEP_FIELD_BUILDER_PLUGIN_STEP_FIELD_BUILDER_DIALOG_H

#include <cnoid/Dialog>
#include <cnoid/ExtensionManager>

namespace cnoid {

class BoxTerrainBuilderDialogImpl;

class BoxTerrainBuilderDialog : public Dialog
{
public:
    BoxTerrainBuilderDialog();
    virtual ~BoxTerrainBuilderDialog();

    static void initialzeClass(ExtensionManager* ext);
    double scale() const;

protected:
    virtual void onAccepted() override;
    virtual void onRejected() override;

private:
    BoxTerrainBuilderDialogImpl* impl;
    friend class BoxTerrainBuilderDialogImpl;
};

}

#endif // CNOID_STEP_FIELD_BUILDER_PLUGIN_STEP_FIELD_BUILDER_DIALOG_H
