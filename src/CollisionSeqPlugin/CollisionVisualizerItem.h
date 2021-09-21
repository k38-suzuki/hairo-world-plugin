/**
   \file
   \author Kenta Suzuki
*/

#ifndef CNOID_COLLISIONSEQPLUGIN_COLLISIONVISUALIZERITEM_H
#define CNOID_COLLISIONSEQPLUGIN_COLLISIONVISUALIZERITEM_H

#include <cnoid/SubSimulatorItem>
#include <vector>

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
    virtual void finalizeSimulation() override;

    void setBodyNames(std::vector<std::string>& bodyNames);
    void setCollisionStatesRecordingEnabled(const bool& on);
    bool collisionStatesRecordingEnabled() const;

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

#endif // CNOID_COLLISIONSEQPLUGIN_COLLISIONVISUALIZERITEM_H
