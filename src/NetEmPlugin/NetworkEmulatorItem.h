/**
   \file
   \author Kenta Suzuki
*/

#ifndef CNOID_NETEMPLUGIN_NETWORKEMULATORITEM_H
#define CNOID_NETEMPLUGIN_NETWORKEMULATORITEM_H

#include <cnoid/SubSimulatorItem>

namespace cnoid {

class NetworkEmulatorItemImpl;

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
    virtual Item* doDuplicate() const override;
    virtual void doPutProperties(PutPropertyFunction& putProperty) override;
    virtual bool store(Archive& archive) override;
    virtual bool restore(const Archive& archive) override;

private:
    NetworkEmulatorItemImpl* impl;
    friend class NetworkEmulatorItemImpl;
};

typedef ref_ptr<NetworkEmulatorItem> NetworkEmulatorItemPtr;

}

#endif // CNOID_NETEMPLUGIN_NETWORKEMULATORITEM_H
