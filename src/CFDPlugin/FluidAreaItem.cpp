/**
   \file
   \author Kenta Suzuki
*/

#include "FluidAreaItem.h"
#include <cnoid/Archive>
#include <cnoid/EigenArchive>
#include <cnoid/FloatingNumberString>
#include <cnoid/ItemManager>
#include <cnoid/PutPropertyFunction>
#include "gettext.h"

using namespace cnoid;
using namespace std;

namespace cnoid {

class FluidAreaItemImpl
{
public:
    FluidAreaItemImpl(FluidAreaItem* self);
    FluidAreaItemImpl(FluidAreaItem* self, const FluidAreaItemImpl& org);
    FluidAreaItem* self;

    FloatingNumberString density;
    FloatingNumberString viscosity;
    Vector3 flow;

    void doPutProperties(PutPropertyFunction& putProperty);
    bool store(Archive& archive);
    bool restore(const Archive& archive);
};

}


FluidAreaItem::FluidAreaItem()
{
    impl = new FluidAreaItemImpl(this);
}


FluidAreaItemImpl::FluidAreaItemImpl(FluidAreaItem* self)
    : self(self)
{
    density = 0.0;
    viscosity = 0.0;
    flow << 0.0, 0.0, 0.0;
}


FluidAreaItem::FluidAreaItem(const FluidAreaItem& org)
    : AreaItem(org),
      impl(new FluidAreaItemImpl(this, *org.impl))
{

}


FluidAreaItemImpl::FluidAreaItemImpl(FluidAreaItem* self, const FluidAreaItemImpl& org)
    : self(self)
{
    density = org.density;
    viscosity = org.viscosity;
    flow = org.flow;
}


FluidAreaItem::~FluidAreaItem()
{
    delete impl;
}


void FluidAreaItem::initializeClass(ExtensionManager* ext)
{
    ext->itemManager().registerClass<FluidAreaItem>(N_("FluidAreaItem"));
    ext->itemManager().addCreationPanel<FluidAreaItem>();
}


void FluidAreaItem::setDensity(const double& density)
{
    impl->density = density;
}


double FluidAreaItem::density() const
{
    return impl->density.value();
}


void FluidAreaItem::setViscosity(const double& viscosity)
{
    impl->viscosity = viscosity;
}


double FluidAreaItem::viscosity() const
{
    return impl->viscosity.value();
}


void FluidAreaItem::setFlow(const Vector3& flow)
{
    impl->flow = flow;
}


Vector3 FluidAreaItem::flow() const
{
    return impl->flow;
}


Item* FluidAreaItem::doDuplicate() const
{
    return new FluidAreaItem(*this);
}


void FluidAreaItem::doPutProperties(PutPropertyFunction& putProperty)
{
    AreaItem::doPutProperties(putProperty);
    impl->doPutProperties(putProperty);
}


void FluidAreaItemImpl::doPutProperties(PutPropertyFunction& putProperty)
{
    putProperty(_("Density"), density,
                [&](const string& v){ return density.setNonNegativeValue(v); });
    putProperty(_("Viscosity"), viscosity,
                [&](const string& v){ return viscosity.setNonNegativeValue(v); });
    putProperty(_("Flow"), str(flow), [&](const string& v){ return toVector3(v, flow); });
}


bool FluidAreaItem::store(Archive& archive)
{
    AreaItem::store(archive);
    return impl->store(archive);
}


bool FluidAreaItemImpl::store(Archive& archive)
{
    archive.write("density", density);
    archive.write("viscosity", viscosity);
    write(archive, "flow", flow);
    return true;
}


bool FluidAreaItem::restore(const Archive &archive)
{
    AreaItem::restore(archive);
    return impl->restore(archive);
}


bool FluidAreaItemImpl::restore(const Archive& archive)
{
    density = archive.get("density", density.string());
    viscosity = archive.get("viscosity", viscosity.string());
    read(archive, "flow", flow);
    return true;
}
