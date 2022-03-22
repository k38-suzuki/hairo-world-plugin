/**
   \file
   \author Kenta Suzuki
*/

#include "VEAreaItem.h"
#include <cnoid/Archive>
#include <cnoid/EigenArchive>
#include <cnoid/EigenUtil>
#include <cnoid/ItemManager>
#include <cnoid/PutPropertyFunction>
#include <cnoid/Selection>
#include "gettext.h"

using namespace cnoid;
using namespace std;

namespace cnoid {

class VEAreaItemImpl
{
public:
    VEAreaItemImpl(VEAreaItem* self);
    VEAreaItemImpl(VEAreaItem* self, const VEAreaItemImpl& org);
    VEAreaItem* self;

    Vector3 hsv;
    Vector3 rgb;
    double coef_b;
    double coef_d;
    double std_dev;
    double salt;
    double pepper;
    bool flip;
    Selection filter;

    bool onPropertyChanged(double& var, const double& v, const double& min, const double& max);
    void doPutProperties(PutPropertyFunction& putProperty);
    bool store(Archive& archive);
    bool restore(const Archive& archive);
};

}


VEAreaItem::VEAreaItem()
{
    impl = new VEAreaItemImpl(this);
}


VEAreaItemImpl::VEAreaItemImpl(VEAreaItem* self)
    : self(self)
{
    self->setDiffuseColor(Vector3(0.0, 1.0, 0.0));
    hsv << 0.0, 0.0, 0.0;
    rgb << 0.0, 0.0, 0.0;
    coef_b = 0.0;
    coef_d = 0.0;
    std_dev = 0.0;
    salt = 0.0;
    pepper = 0.0;
    flip = false;

    static const char* filters[] = { _("No filter"), _("Gaussian 3x3"), _("Gaussian 5x5"), _("Sobel"), _("Prewitt") };
    for(int i = 0; i < 5; ++i) {
        filter.setSymbol(i, filters[i]);
    }
}


VEAreaItem::VEAreaItem(const VEAreaItem& org)
    : AreaItem(org),
      impl(new VEAreaItemImpl(this, *org.impl))
{

}


VEAreaItemImpl::VEAreaItemImpl(VEAreaItem* self, const VEAreaItemImpl& org)
    : self(self)
{
    hsv = org.hsv;
    rgb = org.rgb;
    coef_b = org.coef_b;
    coef_d = org.coef_d;
    std_dev = org.std_dev;
    salt = org.salt;
    pepper = org.pepper;
    flip = org.flip;
    filter = org.filter;
}


VEAreaItem::~VEAreaItem()
{
    delete impl;
}


void VEAreaItem::initializeClass(ExtensionManager* ext)
{
    ext->itemManager().registerClass<VEAreaItem>(N_("VEAreaItem"));
    ext->itemManager().addCreationPanel<VEAreaItem>();
}


Vector3 VEAreaItem::hsv() const
{
    return impl->hsv;
}


Vector3 VEAreaItem::rgb() const
{
    return impl->rgb;
}


double VEAreaItem::coefB() const
{
    return impl->coef_b;
}


double VEAreaItem::coefD() const
{
    return impl->coef_d;
}


double VEAreaItem::stdDev() const
{
    return impl->std_dev;
}


double VEAreaItem::salt() const
{
    return impl->salt;
}


double VEAreaItem::pepper() const
{
    return impl->pepper;
}


bool VEAreaItem::flip() const
{
    return impl->flip;
}


int VEAreaItem::filter() const
{
    return impl->filter;
}


bool VEAreaItemImpl::onPropertyChanged(double& var, const double& v, const double& min, const double& max)
{
    double value = v;
    if(value > max) {
        value = max;
    } else if(value < min) {
        value = min;
    }
    var = value;
    return true;
}


Item* VEAreaItem::doDuplicate() const
{
    return new VEAreaItem(*this);
}


void VEAreaItem::doPutProperties(PutPropertyFunction& putProperty)
{
    AreaItem::doPutProperties(putProperty);
    impl->doPutProperties(putProperty);
}


void VEAreaItemImpl::doPutProperties(PutPropertyFunction& putProperty)
{
    putProperty(_("HSV"), str(hsv), [&](const string& v){ return toVector3(v, hsv); });
    putProperty(_("RGB"), str(rgb), [&](const string& v){ return toVector3(v, rgb); });
    putProperty(_("CoefB"), coef_b,
                [&](const double& v){ return onPropertyChanged(coef_b, v, -1.0, 0.0); });
    putProperty(_("CoefD"), coef_d,
                [&](const double& v){ return onPropertyChanged(coef_d, v, 1.0, 32.0); });
    putProperty(_("Std_dev"), std_dev,
                [&](const double& v){ return onPropertyChanged(std_dev, v, 0.0, 1.0); });
    putProperty(_("Salt"), salt,
                [&](const double& v){ return onPropertyChanged(salt, v, 0.0, 1.0); });
    putProperty(_("Pepper"), pepper,
                [&](const double& v){ return onPropertyChanged(pepper, v, 0.0, 1.0); });
    putProperty(_("Flip"), flip, changeProperty(flip));
    putProperty(_("Filter"), filter,
                [&](const int& index){ return filter.selectIndex(index); });
}


bool VEAreaItem::store(Archive& archive)
{
    AreaItem::store(archive);
    return impl->store(archive);
}


bool VEAreaItemImpl::store(Archive& archive)
{
    write(archive, "hsv", hsv);
    write(archive, "rgb", rgb);
    archive.write("coef_b", coef_b);
    archive.write("coef_d", coef_d);
    archive.write("std_dev", std_dev);
    archive.write("salt", salt);
    archive.write("pepper", pepper);
    archive.write("flip", flip);
    archive.write("filter", filter.selectedIndex());
    return true;
}


bool VEAreaItem::restore(const Archive& archive)
{
    AreaItem::restore(archive);
    return impl->restore(archive);
}


bool VEAreaItemImpl::restore(const Archive& archive)
{
    read(archive, "hsv", hsv);
    read(archive, "rgb", rgb);
    archive.read("coef_b", coef_b);
    archive.read("coef_d", coef_d);
    archive.read("std_dev", std_dev);
    archive.read("salt", salt);
    archive.read("pepper", pepper);
    archive.read("flip", flip);
    filter.selectIndex(archive.get("filter", 0));
    return true;
}
