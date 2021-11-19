/**
   \file
   \author Kenta Suzuki
*/

#ifndef CNOID_BODYGENERATORPLUGIN_TERRAINBUILDERDIALOG_H
#define CNOID_BODYGENERATORPLUGIN_TERRAINBUILDERDIALOG_H

#include <cnoid/Dialog>

namespace cnoid {

class TerrainBuilderDialogImpl;

class TerrainBuilderDialog : public Dialog
{
public:
    TerrainBuilderDialog();
    virtual ~TerrainBuilderDialog();

    static TerrainBuilderDialog* instance();
    double scale() const;

protected:
    virtual void onAccepted();
    virtual void onRejected();

private:
    TerrainBuilderDialogImpl* impl;
    friend class TerrainBuilderDialogImpl;
};

}

#endif // CNOID_BODYGENERATORPLUGIN_TERRAINBUILDERDIALOG_H
