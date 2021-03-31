/**
   \file
   \author Kenta Suzuki
*/

#ifndef CNOID_OBJECT_BUILDER_PLUGIN_OBJECT_BUILDER_DIALOG_H
#define CNOID_OBJECT_BUILDER_PLUGIN_OBJECT_BUILDER_DIALOG_H

#include <cnoid/Dialog>
#include <cnoid/ExtensionManager>

namespace cnoid {

class ObjectBuilderDialogImpl;

class ObjectBuilderDialog : public Dialog
{
public:
    ObjectBuilderDialog();
    virtual ~ObjectBuilderDialog();

    static void initializeClass(ExtensionManager* ext);

protected:
    virtual void onAccepted() override;
    virtual void onRejected() override;

private:
    ObjectBuilderDialogImpl* impl;
    friend class ObjectBuilderDialogImpl;
};

}

#endif // CNOID_OBJECT_BUILDER_PLUGIN_OBJECT_BUILDER_DIALOG_H
