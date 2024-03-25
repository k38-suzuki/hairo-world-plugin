/**
   @author Kenta Suzuki
*/

#include "VFXColliderItem.h"
#include <cnoid/Archive>
#include <cnoid/EigenArchive>
#include <cnoid/EigenUtil>
#include <cnoid/GeneralSliderView>
#include <cnoid/ItemManager>
#include <cnoid/PutPropertyFunction>
#include <cnoid/RootItem>
#include "gettext.h"

using namespace std;
using namespace cnoid;

namespace {

vector<GeneralSliderView::SliderPtr> sliders;

static const double effect_range[11][2] = {
    {  0.0, 1.0 }, { -1.0,  1.0 }, { -1.0, 1.0 },
    { -1.0, 1.0 }, { -1.0,  1.0 }, { -1.0, 1.0 },
    { -1.0, 0.0 }, {  1.0, 32.0 },
    {  0.0, 1.0 }, {  0.0,  1.0 }, {  0.0, 1.0 }
};

void onSelectedItemChanged(const ItemList<>& selectedItems)
{
    GeneralSliderView* sliderView = GeneralSliderView::instance();
    sliders.clear();

    ItemList<VFXColliderItem> areaItems = selectedItems;
    for(auto& areaItem : areaItems) {
        const Vector3& hsv = areaItem->hsv();
        const Vector3& rgb = areaItem->rgb();

        GeneralSliderView::SliderPtr slider_hue = sliderView->getOrCreateSlider(_("Hue"), effect_range[0][0], effect_range[0][1], 2);
        slider_hue->setValue(hsv[0]);
        slider_hue->setCallback([=](double value){
            areaItem->setHsv(Vector3(value, hsv[1], hsv[2]));
            areaItem->notifyUpdate(); });

        GeneralSliderView::SliderPtr slider_saturation = sliderView->getOrCreateSlider(_("Saturation"), effect_range[1][0], effect_range[1][1], 2);
        slider_saturation->setValue(hsv[1]);
        slider_saturation->setCallback([=](double value){
            areaItem->setHsv(Vector3(hsv[0], value, hsv[2]));
            areaItem->notifyUpdate(); });

        GeneralSliderView::SliderPtr slider_value = sliderView->getOrCreateSlider(_("Value"), effect_range[2][0], effect_range[2][1], 2);
        slider_value->setValue(hsv[2]);
        slider_value->setCallback([=](double value){
            areaItem->setHsv(Vector3(hsv[0], hsv[1], value));
            areaItem->notifyUpdate(); });

        GeneralSliderView::SliderPtr slider_red = sliderView->getOrCreateSlider(_("Red"), effect_range[3][0], effect_range[3][1], 2);
        slider_red->setValue(rgb[0]);
        slider_red->setCallback([=](double value){
            areaItem->setRgb(Vector3(value, rgb[1], rgb[2]));
            areaItem->notifyUpdate(); });

        GeneralSliderView::SliderPtr slider_green = sliderView->getOrCreateSlider(_("Green"), effect_range[4][0], effect_range[4][1], 2);
        slider_green->setValue(rgb[1]);
        slider_green->setCallback([=](double value){
            areaItem->setRgb(Vector3(rgb[0], value, rgb[2]));
            areaItem->notifyUpdate(); });

        GeneralSliderView::SliderPtr slider_blue = sliderView->getOrCreateSlider(_("Blue"), effect_range[5][0], effect_range[5][1], 2);
        slider_blue->setValue(rgb[2]);
        slider_blue->setCallback([=](double value){
            areaItem->setRgb(Vector3(rgb[0], rgb[1], value));
            areaItem->notifyUpdate(); });

        GeneralSliderView::SliderPtr slider_coefb = sliderView->getOrCreateSlider(_("CoefB"), effect_range[6][0], effect_range[6][1], 2);
        slider_coefb->setValue(areaItem->coefB());
        slider_coefb->setCallback([=](double value){
            areaItem->setCoefB(value);
            areaItem->notifyUpdate(); });

        GeneralSliderView::SliderPtr slider_coefd = sliderView->getOrCreateSlider(_("CoefD"), effect_range[7][0], effect_range[7][1], 2);
        slider_coefd->setValue(areaItem->coefD());
        slider_coefd->setCallback([=](double value){
            areaItem->setCoefD(value);
            areaItem->notifyUpdate(); });

        GeneralSliderView::SliderPtr slider_stddev = sliderView->getOrCreateSlider(_("Std_dev"), effect_range[8][0], effect_range[8][1], 2);
        slider_stddev->setValue(areaItem->stdDev());
        slider_stddev->setCallback([=](double value){
            areaItem->setStdDev(value);
            areaItem->notifyUpdate(); });

        GeneralSliderView::SliderPtr slider_salt = sliderView->getOrCreateSlider(_("Salt"), effect_range[9][0], effect_range[9][1], 2);
        slider_salt->setValue(areaItem->salt());
        slider_salt->setCallback([=](double value){
            areaItem->setSalt(value);
            areaItem->notifyUpdate(); });

        GeneralSliderView::SliderPtr slider_pepper = sliderView->getOrCreateSlider(_("Pepper"), effect_range[10][0], effect_range[10][1], 2);
        slider_pepper->setValue(areaItem->pepper());
        slider_pepper->setCallback([=](double value){
            areaItem->setPepper(value);
            areaItem->notifyUpdate(); });

        sliders.push_back(slider_hue);
        sliders.push_back(slider_saturation);
        sliders.push_back(slider_value);
        sliders.push_back(slider_red);
        sliders.push_back(slider_green);
        sliders.push_back(slider_blue);
        sliders.push_back(slider_coefb);
        sliders.push_back(slider_coefd);
        sliders.push_back(slider_stddev);
        sliders.push_back(slider_salt);
        sliders.push_back(slider_pepper);
    }
}

}


void VFXColliderItem::initializeClass(ExtensionManager* ext)
{
    ext->itemManager()
        .registerClass<VFXColliderItem>(N_("VFXColliderItem"))
        .addCreationPanel<VFXColliderItem>();

    auto rootItem = RootItem::instance();
    rootItem->sigSelectedItemsChanged().connect(
        [&](const ItemList<>& selectedItems){ onSelectedItemChanged(selectedItems); });
}


VFXColliderItem::VFXColliderItem()
    : SimpleColliderItem(),
      CameraEffects()
{
    setDiffuseColor(Vector3(0.0, 1.0, 0.0));

    hsv_ << 0.0, 0.0, 0.0;
    rgb_ << 0.0, 0.0, 0.0;
    coef_b_ = 0.0;
    coef_d_ = 1.0;
    std_dev_ = 0.0;
    salt_ = 0.0;
    pepper_ = 0.0;
    flipped_ = false;

    filter_.setSymbol(CameraEffects::NO_FILTER, N_("No filter"));
    filter_.setSymbol(CameraEffects::GAUSSIAN_3X3, N_("Gaussian 3x3"));
    filter_.setSymbol(CameraEffects::GAUSSIAN_5X5, N_("Gaussian 5x5"));
    filter_.setSymbol(CameraEffects::SOBEL, N_("Sobel"));
    filter_.setSymbol(CameraEffects::PREWITT, N_("Prewitt"));
}


VFXColliderItem::VFXColliderItem(const VFXColliderItem& org)
    : SimpleColliderItem(org),
      CameraEffects(org)
{
    hsv_ = org.hsv_;
    rgb_ = org.rgb_;
    coef_b_ = org.coef_b_;
    coef_d_ = org.coef_d_;
    std_dev_ = org.std_dev_;
    salt_ = org.salt_;
    pepper_ = org.pepper_;
    flipped_ = org.flipped_;
    filter_ = org.filter_;
}


Item* VFXColliderItem::doCloneItem(CloneMap* cloneMap) const
{
    return new VFXColliderItem(*this);
}


void VFXColliderItem::doPutProperties(PutPropertyFunction& putProperty)
{
    SimpleColliderItem::doPutProperties(putProperty);
    putProperty(_("HSV"), str(hsv_), [&](const string& v){
                    toVector3(v, hsv_);
                    setHsv(hsv_);
                    return true;
                });
    putProperty(_("RGB"), str(rgb_), [&](const string& v){
                    toVector3(v, rgb_);
                    setRgb(rgb_);
                    return true;
                });
    putProperty.min(-1.0).max(0.0)(_("CoefB"), coef_b_,
                [&](const double& value){
                    coef_b_ = value;
                    setCoefB(coef_b_);
                    return true;
                });
    putProperty.min(1.0).max(32.0)(_("CoefD"), coef_d_,
                [&](const double& value){
                    coef_d_ = value;
                    setCoefD(coef_d_);
                    return true;
                });
    putProperty.min(0.0).max(1.0)(_("Std_dev"), std_dev_,
                [&](const double& value){
                    std_dev_ = value;
                    setStdDev(std_dev_);    
                    return true;
                });
    putProperty.min(0.0).max(1.0)(_("Salt"), salt_,
                [&](const double& value){
                    salt_ = value;
                    setSalt(salt_);
                    return true;
                });
    putProperty.min(0.0).max(1.0)(_("Pepper"), pepper_,
                [&](const double& value){
                    pepper_ = value;
                    setPepper(pepper_);
                    return true;
                });
    putProperty(_("Flip"), flipped_, changeProperty(flipped_));
    putProperty(_("Filter"), filter_,
                [&](const int& which){
                    switch(which) {
                    case 0:
                        setFilterType(CameraEffects::NO_FILTER);
                        break;
                    case 1:
                        setFilterType(CameraEffects::GAUSSIAN_3X3);
                        break;
                    case 2:
                        setFilterType(CameraEffects::GAUSSIAN_5X5);
                        break;
                    case 3:
                        setFilterType(CameraEffects::SOBEL);
                        break;
                    case 4:
                        setFilterType(CameraEffects::PREWITT);
                        break;
                    default:
                        break;
                    }
                    return filter_.selectIndex(which);
                });
}


bool VFXColliderItem::store(Archive& archive)
{
    SimpleColliderItem::store(archive);
    write(archive, "hsv", Vector3(hsv_));
    write(archive, "rgb", Vector3(rgb_));
    archive.write("coef_b", coef_b_);
    archive.write("coef_d", coef_d_);
    archive.write("std_dev", std_dev_);
    archive.write("salt", salt_);
    archive.write("pepper", pepper_);
    archive.write("flip", flipped_);
    archive.write("filter", filter_.which());
    return true;
}


bool VFXColliderItem::restore(const Archive& archive)
{
    SimpleColliderItem::restore(archive);
    if(read(archive, "hsv", hsv_)) {
        setHsv(hsv_);
    }
    if(read(archive, "rgb", rgb_)) {
        setRgb(rgb_);
    }
    archive.read("coef_b", coef_b_);
    setCoefB(coef_b_);
    archive.read("coef_d", coef_d_);
    setCoefD(coef_d_);
    archive.read("std_dev", std_dev_);
    setStdDev(std_dev_);
    archive.read("salt", salt_);
    setSalt(salt_);
    archive.read("pepper", pepper_);
    setPepper(pepper_);
    archive.read("flip", flipped_);
    setFlipped(flipped_);
    filter_.selectIndex(archive.get("filter", 0));
    int which = filter_.selectedIndex();
    switch(which) {
    case 0:
        setFilterType(CameraEffects::NO_FILTER);
        break;
    case 1:
        setFilterType(CameraEffects::GAUSSIAN_3X3);
        break;
    case 2:
        setFilterType(CameraEffects::GAUSSIAN_5X5);
        break;
    case 3:
        setFilterType(CameraEffects::SOBEL);
        break;
    case 4:
        setFilterType(CameraEffects::PREWITT);
        break;
    default:
        break;
    }
    return true;
}
