/*!
  @file
  @author Kenta Suzuki
*/

#ifndef CNOID_PHITSPLUGIN_DOSESIMULATORITEM_H
#define CNOID_PHITSPLUGIN_DOSESIMULATORITEM_H

#include <cnoid/SubSimulatorItem>

namespace cnoid {

class DoseSimulatorItemImpl;

class DoseSimulatorItem : public SubSimulatorItem
{
public:
    DoseSimulatorItem();
    DoseSimulatorItem(const DoseSimulatorItem& org);
    virtual ~DoseSimulatorItem();

    static void initializeClass(ExtensionManager* ext);

    virtual bool initializeSimulation(SimulatorItem* simulatorItem);

protected:
    virtual Item* doDuplicate() const;
    virtual void doPutProperties(PutPropertyFunction& putProperty);
    virtual bool store(Archive& archive);
    virtual bool restore(const Archive& archive);

private:
    DoseSimulatorItemImpl* impl;
};

typedef ref_ptr<DoseSimulatorItem> DoseSimulatorItemPtr;

}

#endif