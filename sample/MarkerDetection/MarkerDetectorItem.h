/**
   @author Kenta Suzuki
*/

#ifndef CNOID_MARKERDETECT_PLUGIN_MARKER_DETECTOR_ITEM_H
#define CNOID_MARKERDETECT_PLUGIN_MARKER_DETECTOR_ITEM_H

#include <cnoid/SubSimulatorItem>

namespace cnoid {

class MarkerDetectorItem : public SubSimulatorItem
{
public:
    MarkerDetectorItem();
    MarkerDetectorItem(const MarkerDetectorItem& org);
    virtual ~MarkerDetectorItem();

    static void initializeClass(ExtensionManager* ext);
    virtual bool initializeSimulation(SimulatorItem* simulatorItem) override;

protected:
    virtual Item* doCloneItem(CloneMap* cloneMap) const override;
    virtual void doPutProperties(PutPropertyFunction& putProperty) override;
    virtual bool store(Archive& archive) override;
    virtual bool restore(const Archive& archive) override;

private:
    class Impl;
    Impl* impl;
};

typedef ref_ptr<MarkerDetectorItem> MarkerDetectorItemPtr;

}

#endif // CNOID_MARKERDETECT_PLUGIN_MARKER_DETECTOR_ITEM_H
