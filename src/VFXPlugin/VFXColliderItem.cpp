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
#include <fmt/format.h>
#include "gettext.h"

using namespace std;
using namespace fmt;
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

    filterTypeSelection_.setSymbol(CameraEffects::NO_FILTER, N_("No filter"));
    filterTypeSelection_.setSymbol(CameraEffects::GAUSSIAN_3X3, N_("Gaussian 3x3"));
    filterTypeSelection_.setSymbol(CameraEffects::GAUSSIAN_5X5, N_("Gaussian 5x5"));
    filterTypeSelection_.setSymbol(CameraEffects::SOBEL, N_("Sobel"));
    filterTypeSelection_.setSymbol(CameraEffects::PREWITT, N_("Prewitt"));
}


VFXColliderItem::VFXColliderItem(const VFXColliderItem& org)
    : SimpleColliderItem(org),
      CameraEffects(org)
{
    filterTypeSelection_ = org.filterTypeSelection_;
}


Item* VFXColliderItem::doCloneItem(CloneMap* cloneMap) const
{
    return new VFXColliderItem(*this);
}


void VFXColliderItem::doPutProperties(PutPropertyFunction& putProperty)
{
    SimpleColliderItem::doPutProperties(putProperty);
    putProperty(_("HSV"), format("{0:.3g} {1:.3g} {2:.3g}", hsv().x(), hsv().y(), hsv().z()),
                [this](const string& text){
                    Vector3 hsv;
                    if(toVector3(text, hsv)) {
                        setHsv(hsv);
                        return true;
                    }
                    return false;
                });

    putProperty(_("RGB"),  format("{0:.3g} {1:.3g} {2:.3g}", rgb().x(), rgb().y(), rgb().z()),
                [this](const string& text){
                    Vector3 rgb;
                    if(toVector3(text, rgb)) {
                        setRgb(rgb);
                        return true;                        
                    }
                    return false;
                });

    putProperty.min(-1.0).max(0.0)(_("CoefB"), coefB(),
                [this](double value){
                    setCoefB(value);
                    return true;
                });

    putProperty.min(1.0).max(32.0)(_("CoefD"), coefD(),
                [this](double value){
                    setCoefD(value);
                    return true;
                });

    putProperty.min(0.0).max(1.0)(_("Std_dev"), stdDev(),
                [this](double value){
                    setStdDev(value);    
                    return true;
                });

    putProperty.min(0.0).max(1.0)(_("Salt"), salt(),
                [this](double value){
                    setSalt(value);
                    return true;
                });

    putProperty.min(0.0).max(1.0)(_("Pepper"), pepper(),
                [this](double value){
                    setPepper(value);
                    return true;
                });

    putProperty(_("Flip"), flipped(),
                [this](bool value){
                    setFlipped(value);
                    return true;
                });

    putProperty(_("Filter"), filterTypeSelection_,
                [this](int which){
                    switch(which) {
                    case CameraEffects::NO_FILTER:
                        setFilterType(CameraEffects::NO_FILTER);
                        break;
                    case CameraEffects::GAUSSIAN_3X3:
                        setFilterType(CameraEffects::GAUSSIAN_3X3);
                        break;
                    case CameraEffects::GAUSSIAN_5X5:
                        setFilterType(CameraEffects::GAUSSIAN_5X5);
                        break;
                    case CameraEffects::SOBEL:
                        setFilterType(CameraEffects::SOBEL);
                        break;
                    case CameraEffects::PREWITT:
                        setFilterType(CameraEffects::PREWITT);
                        break;
                    default:
                        break;
                    }
                    return filterTypeSelection_.select(which);
                });
}


bool VFXColliderItem::store(Archive& archive)
{
    SimpleColliderItem::store(archive);
    write(archive, "hsv", Vector3(hsv()));
    write(archive, "rgb", Vector3(rgb()));
    archive.write("coef_b", coefB());
    archive.write("coef_d", coefD());
    archive.write("std_dev", stdDev());
    archive.write("salt", salt());
    archive.write("pepper", pepper());
    archive.write("flipped", flipped());
    archive.write("filter_type", filterTypeSelection_.selectedSymbol());
    return true;
}


bool VFXColliderItem::restore(const Archive& archive)
{
    SimpleColliderItem::restore(archive);
    Vector3 v;
    if(read(archive, "hsv", v)) {
        setHsv(v);
    }
    if(read(archive, "rgb", v)) {
        setRgb(v);
    }
    setCoefB(archive.get("coef_b", 0.0));
    setCoefD(archive.get("coef_d", 1.0));
    setStdDev(archive.get("std_dev", 0.0));
    setSalt(archive.get("salt", 0.0));
    setPepper(archive.get("pepper", 0.0));
    setFlipped(archive.get("flipped", false));
    string type;
    if(archive.read("filter_type", type)) {
        filterTypeSelection_.select(type);
    }
    return true;
}
