/**
   @author Kenta Suzuki
*/

#ifndef CNOID_CFD_PLUGIN_FLUID_COLLIDER_ITEM_H
#define CNOID_CFD_PLUGIN_FLUID_COLLIDER_ITEM_H

#include <cnoid/EigenUtil>
#include <cnoid/SimpleColliderItem>
#include "exportdecl.h"

namespace cnoid {

class CNOID_EXPORT FluidAreaItem : public SimpleColliderItem
{
public:
    static void initializeClass(ExtensionManager* ext);

    FluidAreaItem();
    FluidAreaItem(const FluidAreaItem& org);

    double density() const { return density_; }
    double viscosity() const { return viscosity_; }
    Vector3 steadyFlow() const { return steadyFlow_; }
    void setUnsteadyFlow(const Vector3& unsteadyFlow) { unsteadyFlow_ = unsteadyFlow; }
    Vector3 unsteadyFlow() const { return unsteadyFlow_; }

protected:
    virtual Item* doCloneItem(CloneMap* cloneMap) const override;
    virtual void doPutProperties(PutPropertyFunction& putProperty) override;
    virtual bool store(Archive& archive) override;
    virtual bool restore(const Archive& archive) override;

private:
    double density_;
    double viscosity_;
    Vector3 steadyFlow_;
    Vector3 unsteadyFlow_;
};

typedef ref_ptr<FluidAreaItem> FluidAreaItemPtr;

}

#endif // CNOID_CFD_PLUGIN_FLUID_COLLIDER_ITEM_H
