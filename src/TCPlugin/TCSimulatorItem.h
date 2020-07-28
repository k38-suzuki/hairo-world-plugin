/**
   \file
   \author Kenta Suzuki
*/

#ifndef CNOID_TC_PLUGIN_TC_SIMULATOR_ITEM_H
#define CNOID_TC_PLUGIN_TC_SIMULATOR_ITEM_H

#include <cnoid/SubSimulatorItem>

namespace cnoid {

class TCSimulatorItemImpl;

class TCSimulatorItem : public SubSimulatorItem
{
public:
    TCSimulatorItem();
    TCSimulatorItem(const TCSimulatorItem& org);
    virtual ~TCSimulatorItem();

    static void initializeClass(ExtensionManager* ext);
    virtual bool initializeSimulation(SimulatorItem* simulatorItem) override;
    virtual void finalizeSimulation() override;

protected:
    virtual Item* doDuplicate() const override;
    virtual void doPutProperties(PutPropertyFunction& putProperty) override;
    virtual bool store(Archive& archive) override;
    virtual bool restore(const Archive& archive) override;

private:
    TCSimulatorItemImpl* impl;
    friend class TCSimulatorItemImpl;
};

typedef ref_ptr<TCSimulatorItem> TCSimulatorItemPtr;

}

#endif // CNOID_TC_PLUGIN_TC_SIMULATOR_ITEM_H
