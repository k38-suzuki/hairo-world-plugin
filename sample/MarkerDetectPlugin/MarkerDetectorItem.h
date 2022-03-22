/**
   \file
   \author Kenta Suzuki
*/

#ifndef CNOID_MARKERDETECTPLUGIN_MARKERDETECTORITEM_H
#define CNOID_MARKERDETECTPLUGIN_MARKERDETECTORITEM_H

#include <cnoid/SubSimulatorItem>

namespace cnoid {

class MarkerDetectorItemImpl;

class MarkerDetectorItem : public SubSimulatorItem
{
public:
    MarkerDetectorItem();
    MarkerDetectorItem(const MarkerDetectorItem& org);
    virtual ~MarkerDetectorItem();

    static void initializeClass(ExtensionManager* ext);

    virtual bool initializeSimulation(SimulatorItem* simulatorItem) override;

protected:
    virtual Item* doDuplicate() const override;
    virtual void doPutProperties(PutPropertyFunction& putProperty) override;
    virtual bool store(Archive& archive) override;
    virtual bool restore(const Archive& archive) override;

private:
    MarkerDetectorItemImpl* impl;
    friend class MarkerDetectorItemImpl;
};

typedef ref_ptr<MarkerDetectorItem> MarkerDetectorItemPtr;

}

#endif // CNOID_MARKERDETECTPLUGIN_MARKERDETECTORITEM_H
