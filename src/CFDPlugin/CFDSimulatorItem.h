/**
   @author Kenta Suzuki
*/

#ifndef CNOID_CFD_PLUGIN_CFD_SIMULATOR_ITEM_H
#define CNOID_CFD_PLUGIN_CFD_SIMULATOR_ITEM_H

#include <cnoid/SubSimulatorItem>
#include "exportdecl.h"

namespace cnoid {

class CFDSimulatorItemImpl;

class CNOID_EXPORT CFDSimulatorItem : public SubSimulatorItem
{
public:
    CFDSimulatorItem();
    CFDSimulatorItem(const CFDSimulatorItem& org);
    virtual ~CFDSimulatorItem();

    static void initializeClass(ExtensionManager* ext);
    virtual bool initializeSimulation(SimulatorItem* simulatorItem) override;

protected:
    virtual Item* doCloneItem(CloneMap* cloneMap) const override;
    virtual void doPutProperties(PutPropertyFunction& putProperty) override;
    virtual bool store(Archive& archive) override;
    virtual bool restore(const Archive& archive) override;

private:
    CFDSimulatorItemImpl* impl;
    friend class CFDSimulatorItemImpl;
};

typedef ref_ptr<CFDSimulatorItem> CFDSimulatorItemPtr;

}

#endif // CNOID_CFD_PLUGIN_CFD_SIMULATOR_ITEM_H
