/**
    @author Kenta Suzuki
*/

#ifndef CNOID_MOTION_CAPTURE_PLUGIN_MARKER_COLLISION_VISUALIZER_ITEM_H
#define CNOID_MOTION_CAPTURE_PLUGIN_MARKER_COLLISION_VISUALIZER_ITEM_H

#include <cnoid/CollisionDetectionControllerItem>
#include "exportdecl.h"

namespace cnoid {

class ExtensionManager;

class CNOID_EXPORT CollisionVisualizerItem : public CollisionDetectionControllerItem
{
public:
    static void initializeClass(ExtensionManager* ext);

    CollisionVisualizerItem();
    CollisionVisualizerItem(const CollisionVisualizerItem& org);
    virtual ~CollisionVisualizerItem();

    virtual bool initialize(ControllerIO* io) override;
    virtual bool start() override;
    virtual void input() override;
    virtual bool control() override;
    virtual void output() override;
    virtual void stop() override;

protected:
    virtual Item* doCloneItem(CloneMap* cloneMap) const override;
    virtual void onCollisionsDetected(const std::vector<CollisionLinkPair>& collisions);
    virtual void doPutProperties(PutPropertyFunction& putProperty) override;
    virtual bool store(Archive& archive) override;
    virtual bool restore(const Archive& archive) override;

private:
    class Impl;
    Impl* impl;
};

typedef ref_ptr<CollisionVisualizerItem> CollisionVisualizerItemPtr;

}

#endif // CNOID_MOTION_CAPTURE_PLUGIN_MARKER_COLLISION_VISUALIZER_ITEM_H