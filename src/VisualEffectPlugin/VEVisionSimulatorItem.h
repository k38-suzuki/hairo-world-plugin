/**
   @author Kenta Suzuki
*/

#ifndef CNOID_VISUAL_EFFECT_PLUGIN_VE_VISION_SIMULATOR_ITEM_H
#define CNOID_VISUAL_EFFECT_PLUGIN_VE_VISION_SIMULATOR_ITEM_H

#include <cnoid/GLVisionSimulatorItem>
#include "exportdecl.h"

namespace cnoid {

class CNOID_EXPORT VEVisionSimulatorItem : public GLVisionSimulatorItem
{
public:
    static void initializeClass(ExtensionManager* ext);
        
    VEVisionSimulatorItem();
    VEVisionSimulatorItem(const VEVisionSimulatorItem& org);
    ~VEVisionSimulatorItem();

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

}

#endif
