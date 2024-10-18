/**
   @author Kenta Suzuki
*/

#ifndef CNOID_BOOKMARK_PLUGIN_WORLD_LOG_MANAGER_H
#define CNOID_BOOKMARK_PLUGIN_WORLD_LOG_MANAGER_H

#include "ArchiveListWidget.h"

namespace cnoid {

class CheckBox;
class ExtensionManager;
class SimulatorItem;

class WorldLogManager : public ArchiveListWidget
{
public:
    static void initializeClass(ExtensionManager* ext);

    WorldLogManager(QWidget* parent = nullptr);
    virtual ~WorldLogManager();

protected:
    virtual void onItemDoubleClicked(const std::string& text) override;

private:
    void onSimulationAboutToStart(SimulatorItem* simulatorItem);
    void onPlaybackStopped(double time, bool isStoppedManually);

    CheckBox* autoCheck_;
    CheckBox* saveCheck_;
    bool is_simulation_started_;
    std::string project_filename_;
};

}

#endif // CNOID_BOOKMARK_PLUGIN_WORLD_LOG_MANAGER_H