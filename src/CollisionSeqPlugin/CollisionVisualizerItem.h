/**
   \file
   \author Kenta Suzuki
*/

#ifndef CNOID_COLLISION_SEQ_PLUGIN_COLLISION_VISUALIZER_ITEM_H
#define CNOID_COLLISION_SEQ_PLUGIN_COLLISION_VISUALIZER_ITEM_H

#include <cnoid/SubSimulatorItem>

namespace cnoid {

class CollisionVisualizerItemImpl;

class CollisionVisualizerItem : public SubSimulatorItem
{
public:
    CollisionVisualizerItem();
    CollisionVisualizerItem(const CollisionVisualizerItem& org);
    virtual ~CollisionVisualizerItem();

    static void initializeClass(ExtensionManager* ext);
    virtual bool initializeSimulation(SimulatorItem* simulatorItem) override;

protected:
    virtual Item* doDuplicate() const override;
    virtual void doPutProperties(PutPropertyFunction& putProperty) override;
    virtual bool store(Archive& archive) override;
    virtual bool restore(const Archive& archive) override;

private:
    CollisionVisualizerItemImpl* impl;
    friend class CollisionVisualizerItemImpl;
};

typedef ref_ptr<CollisionVisualizerItem> CollisionVisualizerItemPtr;

}

#endif // CNOID_COLLISION_SEQ_PLUGIN_COLLISION_VISUALIZER_ITEM_H