/**
   \file
   \author Kenta Suzuki
*/

#ifndef CNOID_BODY_GENERATOR_PLUGIN_TERRAIN_GENERATOR_DIALOG_H
#define CNOID_BODY_GENERATOR_PLUGIN_TERRAIN_GENERATOR_DIALOG_H

#include <cnoid/Dialog>

namespace cnoid {

class TerrainGeneratorDialogImpl;

class TerrainGeneratorDialog : public Dialog
{
public:
    TerrainGeneratorDialog();
    virtual ~TerrainGeneratorDialog();

    static TerrainGeneratorDialog* instance();

private:
    TerrainGeneratorDialogImpl* impl;
    friend class TerrainGeneratorDialogImpl;
};

}

#endif // CNOID_BODY_GENERATOR_PLUGIN_TERRAIN_GENERATOR_DIALOG_H