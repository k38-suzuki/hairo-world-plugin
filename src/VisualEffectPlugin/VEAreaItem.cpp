/**
   @author Kenta Suzuki
*/

#include "VEAreaItem.h"
#include <cnoid/Archive>
#include <cnoid/EigenArchive>
#include <cnoid/EigenUtil>
#include <cnoid/GeneralSliderView>
#include <cnoid/ItemManager>
#include <cnoid/PutPropertyFunction>
#include <cnoid/RootItem>
#include "CameraEffect.h"
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

    ItemList<VEAreaItem> areaItems = selectedItems;
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


VEAreaItem::VEAreaItem()
{
    setDiffuseColor(Vector3(0.0, 1.0, 0.0));

    hsv_ << 0.0, 0.0, 0.0;
    rgb_ << 0.0, 0.0, 0.0;
    coef_b_ = 0.0;
    coef_d_ = 1.0;
    std_dev_ = 0.0;
    salt_ = 0.0;
    pepper_ = 0.0;
    flip_ = false;

    filter_.setSymbol(CameraEffect::NO_FILTER, N_("No filter"));
    filter_.setSymbol(CameraEffect::GAUSSIAN_3X3, N_("Gaussian 3x3"));
    filter_.setSymbol(CameraEffect::GAUSSIAN_5X5, N_("Gaussian 5x5"));
    filter_.setSymbol(CameraEffect::SOBEL, N_("Sobel"));
    filter_.setSymbol(CameraEffect::PREWITT, N_("Prewitt"));
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

    auto rootItem = RootItem::instance();
    rootItem->sigSelectedItemsChanged().connect(
        [&](const ItemList<>& selectedItems){ onSelectedItemChanged(selectedItems); });
}


Item* VEAreaItem::doCloneItem(CloneMap* cloneMap) const
{
    return new VEAreaItem(*this);
}


void VEAreaItem::doPutProperties(PutPropertyFunction& putProperty)
{
    AreaItem::doPutProperties(putProperty);
    putProperty(_("HSV"), str(hsv_), [&](const string& v){ return toVector3(v, hsv_); });
    putProperty(_("RGB"), str(rgb_), [&](const string& v){ return toVector3(v, rgb_); });
    putProperty.min(-1.0).max(0.0)(_("CoefB"), coef_b_,
                [&](const double& value){ coef_b_ = value; return true; });
    putProperty.min(1.0).max(32.0)(_("CoefD"), coef_d_,
                [&](const double& value){ coef_d_ = value; return true; });
    putProperty.min(0.0).max(1.0)(_("Std_dev"), std_dev_,
                [&](const double& value){ std_dev_ = value; return true; });
    putProperty.min(0.0).max(1.0)(_("Salt"), salt_,
                [&](const double& value){ salt_ = value; return true; });
    putProperty.min(0.0).max(1.0)(_("Pepper"), pepper_,
                [&](const double& value){ pepper_ = value; return true; });
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
