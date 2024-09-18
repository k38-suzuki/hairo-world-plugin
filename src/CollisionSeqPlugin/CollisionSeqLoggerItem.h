/**
   @author Kenta Suzuki
*/

#ifndef CNOID_COLLISIONSEQ_PLUGIN_COLLISION_SEQ_LOGGER_ITEM_H
#define CNOID_COLLISIONSEQ_PLUGIN_COLLISION_SEQ_LOGGER_ITEM_H

#include <cnoid/SubSimulatorItem>

namespace cnoid {

class CollisionSeqLoggerItem : public SubSimulatorItem
{
public:
    CollisionSeqLoggerItem();
    CollisionSeqLoggerItem(const CollisionSeqLoggerItem& org);
    virtual ~CollisionSeqLoggerItem();

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

typedef ref_ptr<CollisionSeqLoggerItem> CollisionSeqLoggerItemPtr;

}

#endif // CNOID_COLLISIONSEQ_PLUGIN_COLLISION_SEQ_LOGGER_ITEM_H
