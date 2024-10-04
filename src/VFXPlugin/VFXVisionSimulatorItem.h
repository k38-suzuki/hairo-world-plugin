/**
   @author Kenta Suzuki
*/

#ifndef CNOID_VFX_PLUGIN_VE_VISION_SIMULATOR_ITEM_H
#define CNOID_VFX_PLUGIN_VE_VISION_SIMULATOR_ITEM_H

#include <cnoid/GLVisionSimulatorItem>
#include "exportdecl.h"

namespace cnoid {

class CNOID_EXPORT VFXVisionSimulatorItem : public GLVisionSimulatorItem
{
public:
    static void initializeClass(ExtensionManager* ext);

    VFXVisionSimulatorItem();
    VFXVisionSimulatorItem(const VFXVisionSimulatorItem& org);
    virtual ~VFXVisionSimulatorItem();

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

typedef ref_ptr<VFXVisionSimulatorItem> VFXVisionSimulatorItemPtr;

}

#endif // CNOID_VFX_PLUGIN_VE_VISION_SIMULATOR_ITEM_H
