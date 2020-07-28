/**
   \file
   \author Kenta Suzuki
*/

#ifndef CNOID_FLUID_DYNAMICS_PLUGIN_FLUID_DYNAMICS_SIMULATOR_ITEM_H
#define CNOID_FLUID_DYNAMICS_PLUGIN_FLUID_DYNAMICS_SIMULATOR_ITEM_H

#include <cnoid/SubSimulatorItem>

namespace cnoid {

class FluidDynamicsSimulatorItemImpl;

class FluidDynamicsSimulatorItem : public SubSimulatorItem
{
public:
    FluidDynamicsSimulatorItem();
    FluidDynamicsSimulatorItem(const FluidDynamicsSimulatorItem& org);
    ~FluidDynamicsSimulatorItem();

    static void initializeClass(ExtensionManager* ext);
    virtual bool initializeSimulation(SimulatorItem* simulatorItem) override;

protected:
    virtual Item* doDuplicate() const override;
    virtual void doPutProperties(PutPropertyFunction& putProperty) override;
    virtual bool store(Archive& archive) override;
    virtual bool restore(const Archive& archive) override;

private:
    FluidDynamicsSimulatorItemImpl* impl;
    friend class FluidDynamicsSimulatorItemImpl;
};

typedef ref_ptr<FluidDynamicsSimulatorItem> FluidDynamicsSimulatorItemPtr;

}

#endif // CNOID_FLUID_DYNAMICS_PLUGIN_FLUID_DYNAMICS_SIMULATOR_ITEM_H
