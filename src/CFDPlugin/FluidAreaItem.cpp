/**
   \file
   \author Kenta Suzuki
*/

#include "FluidAreaItem.h"
#include <cnoid/Archive>
#include <cnoid/EigenArchive>
#include <cnoid/ItemManager>
#include <cnoid/PutPropertyFunction>
#include "gettext.h"

using namespace cnoid;
using namespace std;

FluidAreaItem::FluidAreaItem()
{
    density_ = 0.0;
    viscosity_ = 0.0;
    flow_ << 0.0, 0.0, 0.0;
}


FluidAreaItem::FluidAreaItem(const FluidAreaItem& org)
    : AreaItem(org)
{
    density_ = org.density_;
    viscosity_ = org.viscosity_;
    flow_ = org.flow_;
}


FluidAreaItem::~FluidAreaItem()
{

}


void FluidAreaItem::initializeClass(ExtensionManager* ext)
{
    ext->itemManager()
            .registerClass<FluidAreaItem>(N_("FluidAreaItem"))
            .addCreationPanel<FluidAreaItem>();
}


Item* FluidAreaItem::doDuplicate() const
{
    return new FluidAreaItem(*this);
}


void FluidAreaItem::doPutProperties(PutPropertyFunction& putProperty)
{
    AreaItem::doPutProperties(putProperty);
    putProperty(_("Density"), density_,
                [&](const string& v){ return density_.setNonNegativeValue(v); });
    putProperty(_("Viscosity"), viscosity_,
                [&](const string& v){ return viscosity_.setNonNegativeValue(v); });
    putProperty(_("Flow"), str(flow_), [&](const string& v){ return toVector3(v, flow_); });
}


bool FluidAreaItem::store(Archive& archive)
{
    AreaItem::store(archive);
    archive.write("density", density_);
    archive.write("viscosity", viscosity_);
    write(archive, "flow", flow_);
    return true;
}


bool FluidAreaItem::restore(const Archive &archive)
{
    AreaItem::restore(archive);
    density_ = archive.get("density", density_.string());
    viscosity_ = archive.get("viscosity", viscosity_.string());
    read(archive, "flow", flow_);
    return true;
}
