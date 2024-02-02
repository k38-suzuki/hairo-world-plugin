/**
   @author Kenta Suzuki
*/

#ifndef CNOID_CFD_PLUGIN_FLUID_AREA_ITEM_H
#define CNOID_CFD_PLUGIN_FLUID_AREA_ITEM_H

#include "AreaItem.h"
#include <cnoid/FloatingNumberString>
#include "exportdecl.h"

namespace cnoid {

class CNOID_EXPORT FluidAreaItem : public AreaItem
{
public:
    FluidAreaItem();
    FluidAreaItem(const FluidAreaItem& org);
    virtual ~FluidAreaItem();

    static void initializeClass(ExtensionManager* ext);

    double density() const { return density_.value(); }
    double viscosity() const { return viscosity_.value(); }
    Vector3 steadyFlow() const { return steadyFlow_; }
    void setUnsteadyFlow(const Vector3& unsteadyFlow) { unsteadyFlow_ = unsteadyFlow; }
    Vector3 unsteadyFlow() const { return unsteadyFlow_; }

protected:
    virtual Item* doDuplicate() const override;
    virtual void doPutProperties(PutPropertyFunction& putProperty) override;
    virtual bool store(Archive& archive) override;
    virtual bool restore(const Archive& archive) override;

private:
    FloatingNumberString density_;
    FloatingNumberString viscosity_;
    Vector3 steadyFlow_;
    Vector3 unsteadyFlow_;
};

typedef ref_ptr<FluidAreaItem> FluidAreaItemPtr;

}

#endif
