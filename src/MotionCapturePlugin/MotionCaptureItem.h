/**
   @author Kenta Suzuki
*/

#ifndef CNOID_MOTIONCAPTURE_PLUGIN_MOTION_CAPTURE_ITEM_H
#define CNOID_MOTIONCAPTURE_PLUGIN_MOTION_CAPTURE_ITEM_H

#include <cnoid/SubSimulatorItem>

namespace cnoid {

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
    virtual Item* doCloneItem(CloneMap* cloneMap) const override;
    virtual void doPutProperties(PutPropertyFunction& putProperty) override;
    virtual bool store(Archive& archive) override;
    virtual bool restore(const Archive& archive) override;

private:
    class Impl;
    Impl* impl;
};

typedef ref_ptr<MotionCaptureItem> MotionCaptureItemPtr;

}

#endif // CNOID_MOTIONCAPTURE_PLUGIN_MOTION_CAPTURE_ITEM_H