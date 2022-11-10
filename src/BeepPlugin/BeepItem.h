/**
   \file
   \author Kenta Suzuki
*/

#ifndef CNOID_BEEPPLUGIN_BEEPITEM_H
#define CNOID_BEEPPLUGIN_BEEPITEM_H

#include <cnoid/SubSimulatorItem>

namespace cnoid {

class BeepItemImpl;

class BeepItem : public SubSimulatorItem
{
public:
    BeepItem();
    BeepItem(const BeepItem& org);
    virtual ~BeepItem();

    static void initializeClass(ExtensionManager* ext);
    virtual bool initializeSimulation(SimulatorItem* simulatorItem) override;

protected:
    virtual Item* doDuplicate() const override;
    virtual void doPutProperties(PutPropertyFunction& putProperty) override;
    virtual bool store(Archive& archive) override;
    virtual bool restore(const Archive& archive) override;

private:
    BeepItemImpl* impl;
    friend class BeepItemImpl;
};

typedef ref_ptr<BeepItem> BeepItemPtr;

}

#endif // CNOID_BEEPPLUGIN_BEEPITEM_H
