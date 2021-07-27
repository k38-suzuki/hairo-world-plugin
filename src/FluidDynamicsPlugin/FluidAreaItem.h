/**
   \file
   \author Kenta Suzuki
*/

#ifndef CNOID_FLUIDDYNAMICSPLUGIN_FLUIDAREAITEM_H
#define CNOID_FLUIDDYNAMICSPLUGIN_FLUIDAREAITEM_H

#include "AreaItem.h"

namespace cnoid {

class FluidAreaItemImpl;

class FluidAreaItem : public AreaItem
{
public:
    FluidAreaItem();
    FluidAreaItem(const FluidAreaItem& org);
    virtual ~FluidAreaItem();

    static void initializeClass(ExtensionManager* ext);

    void setDensity(const double& density);
    double density() const;
    void setViscosity(const double& viscosity);
    double viscosity() const;
    void setFlow(const Vector3& flow);
    Vector3 flow() const;

    static bool load(FluidAreaItem* item, const std::string& filename);
    static bool save(FluidAreaItem* item, const std::string& filename);

protected:
    virtual Item* doDuplicate() const override;
    virtual void doPutProperties(PutPropertyFunction& putProperty) override;
    virtual bool store(Archive& archive) override;
    virtual bool restore(const Archive& archive) override;

private:
    FluidAreaItemImpl* impl;
    friend class FluidAreaItemImpl;
};

typedef ref_ptr<FluidAreaItem> FluidAreaItemPtr;

}

#endif // CNOID_FLUIDDYNAMICSPLUGIN_FLUIDAREAITEM_H
