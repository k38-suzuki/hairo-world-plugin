/**
   \file
   \author Kenta Suzuki
*/

#ifndef CNOID_BOXTERRAINBUILDERPLUGIN_BOXTERRAINBUILDERDIALOG_H
#define CNOID_BOXTERRAINBUILDERPLUGIN_BOXTERRAINBUILDERDIALOG_H

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
    static BoxTerrainBuilderDialog* instance();

    double scale() const;

protected:
    virtual void onAccepted() override;
    virtual void onRejected() override;

private:
    BoxTerrainBuilderDialogImpl* impl;
    friend class BoxTerrainBuilderDialogImpl;
};

}

#endif // CNOID_BOXTERRAINBUILDERPLUGIN_BOXTERRAINBUILDERDIALOG_H
