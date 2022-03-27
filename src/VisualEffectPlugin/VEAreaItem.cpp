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
#include "gettext.h"

using namespace cnoid;
using namespace std;

namespace {

bool onPropertyChanged(double& var, const double& v, const double& min, const double& max)
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

}


VEAreaItem::VEAreaItem()
{
    setDiffuseColor(Vector3(0.0, 1.0, 0.0));
    hsv_ << 0.0, 0.0, 0.0;
    rgb_ << 0.0, 0.0, 0.0;
    coef_b_ = 0.0;
    coef_d_ = 0.0;
    std_dev_ = 0.0;
    salt_ = 0.0;
    pepper_ = 0.0;
    flip_ = false;

    static const char* filters[] = {
        _("No filter"), _("Gaussian 3x3"), _("Gaussian 5x5"), _("Sobel"), _("Prewitt") };
    for(int i = 0; i < 5; ++i) {
        filter_.setSymbol(i, filters[i]);
    }
}


VEAreaItem::VEAreaItem(const VEAreaItem& org)
    : AreaItem(org)
{
    hsv_ = org.hsv_;
    rgb_ = org.rgb_;
    coef_b_ = org.coef_b_;
    coef_d_ = org.coef_d_;
    std_dev_ = org.std_dev_;
    salt_ = org.salt_;
    pepper_ = org.pepper_;
    flip_ = org.flip_;
    filter_ = org.filter_;
}


VEAreaItem::~VEAreaItem()
{

}


void VEAreaItem::initializeClass(ExtensionManager* ext)
{
    ext->itemManager()
            .registerClass<VEAreaItem>(N_("VEAreaItem"))
            .addCreationPanel<VEAreaItem>();
}


Item* VEAreaItem::doDuplicate() const
{
    return new VEAreaItem(*this);
}


void VEAreaItem::doPutProperties(PutPropertyFunction& putProperty)
{
    AreaItem::doPutProperties(putProperty);
    putProperty(_("HSV"), str(hsv_), [&](const string& v){ return toVector3(v, hsv_); });
    putProperty(_("RGB"), str(rgb_), [&](const string& v){ return toVector3(v, rgb_); });
    putProperty(_("CoefB"), coef_b_,
                [&](const double& v){ return onPropertyChanged(coef_b_, v, -1.0, 0.0); });
    putProperty(_("CoefD"), coef_d_,
                [&](const double& v){ return onPropertyChanged(coef_d_, v, 1.0, 32.0); });
    putProperty(_("Std_dev"), std_dev_,
                [&](const double& v){ return onPropertyChanged(std_dev_, v, 0.0, 1.0); });
    putProperty(_("Salt"), salt_,
                [&](const double& v){ return onPropertyChanged(salt_, v, 0.0, 1.0); });
    putProperty(_("Pepper"), pepper_,
                [&](const double& v){ return onPropertyChanged(pepper_, v, 0.0, 1.0); });
    putProperty(_("Flip"), flip_, changeProperty(flip_));
    putProperty(_("Filter"), filter_,
                [&](const int& index){ return filter_.selectIndex(index); });
}


bool VEAreaItem::store(Archive& archive)
{
    AreaItem::store(archive);
    write(archive, "hsv", hsv_);
    write(archive, "rgb", rgb_);
    archive.write("coef_b", coef_b_);
    archive.write("coef_d", coef_d_);
    archive.write("std_dev", std_dev_);
    archive.write("salt", salt_);
    archive.write("pepper", pepper_);
    archive.write("flip", flip_);
    archive.write("filter", filter_.which());
    return true;
}


bool VEAreaItem::restore(const Archive& archive)
{
    AreaItem::restore(archive);
    read(archive, "hsv", hsv_);
    read(archive, "rgb", rgb_);
    archive.read("coef_b", coef_b_);
    archive.read("coef_d", coef_d_);
    archive.read("std_dev", std_dev_);
    archive.read("salt", salt_);
    archive.read("pepper", pepper_);
    archive.read("flip", flip_);
    filter_.selectIndex(archive.get("filter", 0));
    return true;
}
