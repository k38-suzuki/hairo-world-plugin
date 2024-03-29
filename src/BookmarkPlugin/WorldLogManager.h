/**
   @author Kenta Suzuki
*/

#ifndef CNOID_BOOKMARK_PLUGIN_WORLD_LOG_MANAGER_H
#define CNOID_BOOKMARK_PLUGIN_WORLD_LOG_MANAGER_H

#include "ArchiveListDialog.h"

namespace cnoid {

class CheckBox;
class ExtensionManager;
class SimulatorItem;

class WorldLogManager : public ArchiveListDialog
{
public:
    static void initializeClass(ExtensionManager* ext);
    static WorldLogManager* instance();

    WorldLogManager();
    virtual ~WorldLogManager();

protected:
    virtual void onItemDoubleClicked(std::string& text) override;

private:
    CheckBox* saveCheck;
    std::string project_filename;
    bool is_started;
    void onSimulationAboutToStart(SimulatorItem* simulatorItem);
    void onPlaybackStopped(double time, bool isStoppedManually);
};

}

#endif // CNOID_BOOKMARK_PLUGIN_WORLD_LOG_MANAGER_H
