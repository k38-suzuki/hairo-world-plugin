/**
   \file
   \author Kenta Suzuki
*/

#ifndef CNOID_CFDPLUGIN_CFDSIMULATORITEM_H
#define CNOID_CFDPLUGIN_CFDSIMULATORITEM_H

#include <cnoid/SubSimulatorItem>

namespace cnoid {

class CFDSimulatorItemImpl;

class CFDSimulatorItem : public SubSimulatorItem
{
public:
    CFDSimulatorItem();
    CFDSimulatorItem(const CFDSimulatorItem& org);
    virtual ~CFDSimulatorItem();

    static void initializeClass(ExtensionManager* ext);
    virtual bool initializeSimulation(SimulatorItem* simulatorItem) override;

protected:
    virtual Item* doDuplicate() const override;
    virtual void doPutProperties(PutPropertyFunction& putProperty) override;
    virtual bool store(Archive& archive) override;
    virtual bool restore(const Archive& archive) override;

private:
    CFDSimulatorItemImpl* impl;
    friend class CFDSimulatorItemImpl;
};

typedef ref_ptr<CFDSimulatorItem> CFDSimulatorItemPtr;

}

#endif // CNOID_CFDPLUGIN_CFDSIMULATORITEM_H
