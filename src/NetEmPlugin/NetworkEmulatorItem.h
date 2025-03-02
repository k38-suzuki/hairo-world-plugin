/**
   @author Kenta Suzuki
*/

#ifndef CNOID_NETEM_PLUGIN_NETWORK_EMULATOR_ITEM_H
#define CNOID_NETEM_PLUGIN_NETWORK_EMULATOR_ITEM_H

#include <cnoid/SubSimulatorItem>

namespace cnoid {

class NetworkEmulatorItem : public SubSimulatorItem
{
public:
    NetworkEmulatorItem();
    NetworkEmulatorItem(const NetworkEmulatorItem& org);
    virtual ~NetworkEmulatorItem();

    static void initializeClass(ExtensionManager* ext);
    virtual bool initializeSimulation(SimulatorItem* simulatorItem) override;
    virtual void finalizeSimulation() override;

protected:
    virtual Item* doCloneItem(CloneMap* cloneMap) const override;
    virtual void doPutProperties(PutPropertyFunction& putProperty) override;
    virtual bool store(Archive& archive) override;
    virtual bool restore(const Archive& archive) override;

private:
    class Impl;
    Impl* impl;
};

typedef ref_ptr<NetworkEmulatorItem> NetworkEmulatorItemPtr;

}

#endif // CNOID_NETEM_PLUGIN_NETWORK_EMULATOR_ITEM_H
