/**
   @author Kenta Suzuki
*/

#include "FluidAreaItem.h"
#include <cnoid/Archive>
#include <cnoid/EigenArchive>
#include <cnoid/ItemManager>
#include <cnoid/PutPropertyFunction>
#include "gettext.h"

using namespace std;
using namespace cnoid;

void FluidAreaItem::initializeClass(ExtensionManager* ext)
{
    ext->itemManager()
        .registerClass<FluidAreaItem>(N_("FluidAreaItem"))
        .addCreationPanel<FluidAreaItem>();
}


FluidAreaItem::FluidAreaItem()
    : SimpleColliderItem()
{
    density_ = 0.0;
    viscosity_ = 0.0;
    steadyFlow_ << 0.0, 0.0, 0.0;
    unsteadyFlow_ << 0.0, 0.0, 0.0;
}


FluidAreaItem::FluidAreaItem(const FluidAreaItem& org)
    : SimpleColliderItem(org)
{
    density_ = org.density_;
    viscosity_ = org.viscosity_;
    steadyFlow_ = org.steadyFlow_;
    unsteadyFlow_ = org.unsteadyFlow_;
}


Item* FluidAreaItem::doCloneItem(CloneMap* cloneMap) const
{
    return new FluidAreaItem(*this);
}


void FluidAreaItem::doPutProperties(PutPropertyFunction& putProperty)
{
    SimpleColliderItem::doPutProperties(putProperty);
    putProperty.min(0.0).max(9999.0)(_("Density"), density_, changeProperty(density_));
    putProperty.min(0.0).max(9999.0)(_("Viscosity"), viscosity_, changeProperty(viscosity_));
    putProperty(_("SteadyFlow"), str(steadyFlow_),
                [this](const string& text){
                    Vector3 flow;
                    if(toVector3(text, flow)) {
                        steadyFlow_ = flow;
                        return true;
                    }
                    return false;
                });
}


bool FluidAreaItem::store(Archive& archive)
{
    SimpleColliderItem::store(archive);
    archive.write("density", density_);
    archive.write("viscosity", viscosity_);
    write(archive, "steady_flow", Vector3(steadyFlow_));
    return true;
}


bool FluidAreaItem::restore(const Archive &archive)
{
    SimpleColliderItem::restore(archive);
    archive.read("density", density_);
    archive.read("viscosity", viscosity_);
    Vector3 flow;
    if(read(archive, "steady_flow", flow)) {
        steadyFlow_ = flow;
    }
    return true;
}
