/**
   \file
   \author Kenta Suzuki
*/

#include "VEAreaItem.h"
#include <cnoid/Archive>
#include <cnoid/EigenTypes>
#include <cnoid/EigenUtil>
#include <cnoid/FloatingNumberString>
#include <cnoid/ItemManager>
#include <cnoid/PutPropertyFunction>
#include <cnoid/Selection>
#include "gettext.h"

using namespace cnoid;
using namespace std;

namespace {

template<class VectorType>
static bool toVectorX_(const std::string& s, VectorType& out_v)
{
    const char* nptr = s.c_str();
    char* endptr;
    for(int i = 0; i < out_v.rows(); ++i) {
        out_v[i] = strtod(nptr, &endptr);
        if(endptr == nptr) {
            return false;
        }
        nptr = endptr;
        while(isspace(*nptr)) {
            nptr++;
        }
        if(*nptr == ',') {
            nptr++;
        }
    }
    return true;
}

}


namespace cnoid {

class VEAreaItemImpl
{
public:
    VEAreaItemImpl(VEAreaItem* self);
    VEAreaItemImpl(VEAreaItem* self, const VEAreaItemImpl& org);
    VEAreaItem* self;

    double hue;
    double saturation;
    double value;
    double red;
    double green;
    double blue;
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
    hue = 0.0;
    saturation = 0.0;
    value = 0.0;
    red = 0.0;
    green = 0.0;
    blue = 0.0;
    coef_b = 0.0;
    coef_d = 0.0;
    std_dev = 0.0;
    salt = 0.0;
    pepper = 0.0;
    flip = false;

    const char* filters[] = { _("No filter"), _("Gaussian 3x3"), _("Gaussian 5x5"), _("Sobel"), _("Prewitt") };
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
    hue = org.hue;
    saturation = org.saturation;
    value = org.value;
    red = org.red;
    green = org.green;
    blue = org.blue;
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
    ItemManager& im = ext->itemManager();
    im.registerClass<VEAreaItem>(N_("VEAreaItem"));
    im.addCreationPanel<VEAreaItem>();

//    im.addLoaderAndSaver<VEAreaItem>(
//        _("VF Area"), "VF-AREA-FILE", "yaml;yml",
//        [](VEAreaItem* item, const string& filename, std::ostream& os, Item*){ return load(item, filename); },
//        [](VEAreaItem* item, const string& filename, std::ostream& os, Item*){ return save(item, filename); },
//        ItemManager::PRIORITY_CONVERSION);
}


double VEAreaItem::hue() const
{
    return impl->hue;
}


double VEAreaItem::saturation() const
{
    return impl->saturation;
}


double VEAreaItem::value() const
{
    return impl->value;
}


double VEAreaItem::red() const
{
    return impl->red;
}


double VEAreaItem::green() const
{
    return impl->green;
}


double VEAreaItem::blue() const
{
    return impl->blue;
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


bool VEAreaItem::load(VEAreaItem* item, const string& filename)
{
    return true;
}


bool VEAreaItem::save(VEAreaItem* item, const string& filename)
{
    return true;
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
    impl->doPutProperties(putProperty);
    AreaItem::doPutProperties(putProperty);
}


void VEAreaItemImpl::doPutProperties(PutPropertyFunction& putProperty)
{
    putProperty(_("Hue"), hue,
                [&](const double& v){ return onPropertyChanged(hue, v, 0.0, 1.0); });
    putProperty(_("Saturation"), saturation,
                [&](const double& v){ return onPropertyChanged(saturation, v, 0.0, 1.0); });
    putProperty(_("Value"), value,
                [&](const double& v){ return onPropertyChanged(value, v, 0.0, 1.0); });
    putProperty(_("Red"), red,
                [&](const double& v){ return onPropertyChanged(red, v, 0.0, 1.0); });
    putProperty(_("Green"), green,
                [&](const double& v){ return onPropertyChanged(green, v, 0.0, 1.0); });
    putProperty(_("Blue"), blue,
                [&](const double& v){ return onPropertyChanged(blue, v, 0.0, 1.0); });
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
    archive.write("hue", hue);
    archive.write("saturation", saturation);
    archive.write("value", value);
    archive.write("red", red);
    archive.write("green", green);
    archive.write("blue", blue);
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
    archive.read("hue", hue);
    archive.read("saturation", saturation);
    archive.read("value", value);
    archive.read("red", red);
    archive.read("green", green);
    archive.read("blue", blue);
    archive.read("coef_b", coef_b);
    archive.read("coef_d", coef_d);
    archive.read("std_dev", std_dev);
    archive.read("salt", salt);
    archive.read("pepper", pepper);
    archive.read("flip", flip);

    int i = 0;
    archive.read("filter", i);
    filter.selectIndex(i);
    return true;
}
