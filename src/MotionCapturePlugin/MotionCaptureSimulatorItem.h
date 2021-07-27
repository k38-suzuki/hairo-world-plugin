/**
   \file
   \author Kenta Suzuki
*/

#ifndef CNOID_MOTIONCAPTUREPLUGIN_MOTIONCAPTURESIMULATORITEM_H
#define CNOID_MOTIONCAPTUREPLUGIN_MOTIONCAPTURESIMULATORITEM_H

#include <cnoid/SubSimulatorItem>

namespace cnoid {

class MotionCaptureSimulatorItemImpl;

class MotionCaptureSimulatorItem : public SubSimulatorItem
{
public:
    MotionCaptureSimulatorItem();
    MotionCaptureSimulatorItem(const MotionCaptureSimulatorItem& org);
    ~MotionCaptureSimulatorItem();

    static void initializeClass(ExtensionManager* ext);

    virtual bool initializeSimulation(SimulatorItem* simulatorItem) override;
    virtual void finalizeSimulation() override;

protected:
    virtual Item* doDuplicate() const override;
    virtual void doPutProperties(PutPropertyFunction& putProperty) override;
    virtual bool store(Archive& archive) override;
    virtual bool restore(const Archive& archive) override;

private:
    MotionCaptureSimulatorItemImpl* impl;
    friend class MotionCaptureSimulatorItemImpl;
};

typedef ref_ptr<MotionCaptureSimulatorItem> MotionCaptureSimulatorItemPtr;

}

#endif // CNOID_MOTIONCAPTUREPLUGIN_MOTIONCAPTURESIMULATORITEM_H
