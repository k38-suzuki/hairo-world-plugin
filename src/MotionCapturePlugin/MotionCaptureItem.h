/**
   \file
   \author Kenta Suzuki
*/

#ifndef CNOID_MOTIONCAPTUREPLUGIN_MOTIONCAPTUREITEM_H
#define CNOID_MOTIONCAPTUREPLUGIN_MOTIONCAPTUREITEM_H

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

#endif // CNOID_MOTIONCAPTUREPLUGIN_MOTIONCAPTUREITEM_H
