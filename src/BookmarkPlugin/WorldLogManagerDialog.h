/**
   \file
   \author Kenta Suzuki
*/

#ifndef CNOID_BOOKMARK_PLUGIN_WORLD_LOG_MANAGER_H
#define CNOID_BOOKMARK_PLUGIN_WORLD_LOG_MANAGER_H

#include <cnoid/Dialog>

namespace cnoid {

class WorldLogManagerDialogImpl;

class WorldLogManagerDialog : public Dialog
{
public:
    WorldLogManagerDialog();
    virtual ~WorldLogManagerDialog();

    static WorldLogManagerDialog* instance();

    void showWorldLogManagerDialog();

private:
    WorldLogManagerDialogImpl* impl;
    friend class WorldLogManagerDialogImpl;
};

}

#endif // CNOID_BOOKMARK_PLUGIN_WORLD_LOG_MANAGER_H