/**
   @author Kenta Suzuki
*/

#ifndef CNOID_PHITS_PLUGIN_GAMMA_VISION_SIMULATOR_ITEM_H
#define CNOID_PHITS_PLUGIN_GAMMA_VISION_SIMULATOR_ITEM_H

#include <cnoid/GLVisionSimulatorItem>
#include "exportdecl.h"

namespace cnoid {

class CNOID_EXPORT GammaVisionSimulatorItem : public GLVisionSimulatorItem
{
public:
    static void initializeClass(ExtensionManager* ext);
        
    GammaVisionSimulatorItem();
    GammaVisionSimulatorItem(const GammaVisionSimulatorItem& org);
    ~GammaVisionSimulatorItem();

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

}

#endif
