/**
   @author Kenta Suzuki
*/

#ifndef CNOID_COLLISIONSEQ_PLUGIN_COLLISION_VISUALIZER_ITEM_H
#define CNOID_COLLISIONSEQ_PLUGIN_COLLISION_VISUALIZER_ITEM_H

#include <cnoid/SubSimulatorItem>

namespace cnoid {

class CollisionLoggerItem : public SubSimulatorItem
{
public:
    CollisionLoggerItem();
    CollisionLoggerItem(const CollisionLoggerItem& org);
    virtual ~CollisionLoggerItem();

    static void initializeClass(ExtensionManager* ext);
    virtual bool initializeSimulation(SimulatorItem* simulatorItem) override;

protected:
    virtual Item* doCloneItem(CloneMap* cloneMap) const override;
    virtual void doPutProperties(PutPropertyFunction& putProperty) override;
    virtual bool store(Archive& archive) override;
    virtual bool restore(const Archive& archive) override;

private:
    class Impl;
    Impl* impl;
};

typedef ref_ptr<CollisionLoggerItem> CollisionLoggerItemPtr;

}

#endif // CNOID_COLLISIONSEQ_PLUGIN_COLLISION_VISUALIZER_ITEM_H
