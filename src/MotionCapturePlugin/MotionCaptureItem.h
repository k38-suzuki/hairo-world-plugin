/**
   \file
   \author Kenta Suzuki
*/

#ifndef CNOID_MOTION_CAPTURE_PLUGIN_MOTION_CAPTURE_ITEM_H
#define CNOID_MOTION_CAPTURE_PLUGIN_MOTION_CAPTURE_ITEM_H

#include <cnoid/SubSimulatorItem>

namespace cnoid {

class MotionCaptureItemImpl;

class MotionCaptureItem : public SubSimulatorItem
{
public:
    MotionCaptureItem();
    MotionCaptureItem(const MotionCaptureItem& org);
    virtual ~MotionCaptureItem();

    static void initializeClass(ExtensionManager* ext);

    virtual bool initializeSimulation(SimulatorItem* simulatorItem) override;
    virtual void finalizeSimulation() override;

protected:
    virtual Item* doDuplicate() const override;
    virtual void doPutProperties(PutPropertyFunction& putProperty) override;
    virtual bool store(Archive& archive) override;
    virtual bool restore(const Archive& archive) override;

private:
    MotionCaptureItemImpl* impl;
    friend class MotionCaptureItemImpl;
};

}

#endif // CNOID_MOTION_CAPTURE_PLUGIN_MOTION_CAPTURE_ITEM_H