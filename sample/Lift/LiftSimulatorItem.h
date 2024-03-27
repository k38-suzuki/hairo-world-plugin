/**
   @author Kenta Suzuki
*/

#ifndef CNOID_SAMPLE_LIFT_SIMULATOR_ITEM_H
#define CNOID_SAMPLE_LIFT_SIMULATOR_ITEM_H

#include <cnoid/DeviceList>
#include <cnoid/ItemList>
#include <cnoid/SimulatorItem>
#include <cnoid/SubSimulatorItem>
#include <cnoid/MultiColliderItem>
#include <cnoid/WingDevice>

namespace cnoid {

class LiftSimulatorItem : public SubSimulatorItem
{
public:
    LiftSimulatorItem();
    LiftSimulatorItem(const LiftSimulatorItem& org);

    static void initializeClass(ExtensionManager* ext);
    virtual bool initializeSimulation(SimulatorItem* simulatorItem) override;

protected:
    virtual Item* doCloneItem(CloneMap* cloneMap) const override;
    virtual void doPutProperties(PutPropertyFunction& putProperty) override;
    virtual bool store(Archive& archive) override;
    virtual bool restore(const Archive& archive) override;

private:
    SimulatorItem* simulatorItem;
    DeviceList<WingDevice> wings;
    ItemList<MultiColliderItem> colliders;

    void onPreDynamics();
};

typedef ref_ptr<LiftSimulatorItem> LiftSimulatorItemPtr;

}

#endif // CNOID_SAMPLE_LIFT_SIMULATOR_ITEM_H
