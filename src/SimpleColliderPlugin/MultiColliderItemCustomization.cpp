/**
   @author Kenta Suzuki
*/

#include "MultiColliderItem.h"
#include <cnoid/ExtensionManager>
#include <cnoid/ItemManager>
#include <cnoid/RootItem>
#include <cnoid/GeneralSliderView>
#include <QBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include "gettext.h"

using namespace std;
using namespace cnoid;

namespace {

vector<GeneralSliderView::SliderPtr> sliders;

static const double effect_range[11][2] = {
    {  0.0, 1.0 }, { 0.0,  1.0 }, { 0.0, 1.0 },
    {  0.0, 1.0 }, { 0.0,  1.0 }, { 0.0, 1.0 },
    { -1.0, 0.0 }, { 1.0, 32.0 },
    {  0.0, 1.0 }, { 0.0,  1.0 }, { 0.0, 1.0 }
};

void onSelectedItemChanged(const ItemList<>& selectedItems)
{
    GeneralSliderView* sliderView = GeneralSliderView::instance();
    sliders.clear();

    ItemList<MultiColliderItem> colliders = selectedItems;
    for(auto& collider : colliders) {
        const Vector3& hsv = collider->hsv();
        const Vector3& rgb = collider->rgb();

        GeneralSliderView::SliderPtr slider_hue = sliderView->getOrCreateSlider(_("hue"), effect_range[0][0], effect_range[0][1], 2);
        slider_hue->setValue(hsv[0]);
        slider_hue->setCallback([collider, hsv](double value){
            collider->setHsv(Vector3(value, hsv[1], hsv[2]));
            collider->notifyUpdate(); });

        GeneralSliderView::SliderPtr slider_saturation = sliderView->getOrCreateSlider(_("saturation"), effect_range[1][0], effect_range[1][1], 2);
        slider_saturation->setValue(hsv[1]);
        slider_saturation->setCallback([collider, hsv](double value){
            collider->setHsv(Vector3(hsv[0], value, hsv[2]));
            collider->notifyUpdate(); });

        GeneralSliderView::SliderPtr slider_value = sliderView->getOrCreateSlider(_("value"), effect_range[2][0], effect_range[2][1], 2);
        slider_value->setValue(hsv[2]);
        slider_value->setCallback([collider, hsv](double value){
            collider->setHsv(Vector3(hsv[0], hsv[1], value));
            collider->notifyUpdate(); });

        GeneralSliderView::SliderPtr slider_red = sliderView->getOrCreateSlider(_("red"), effect_range[3][0], effect_range[3][1], 2);
        slider_red->setValue(rgb[0]);
        slider_red->setCallback([collider, rgb](double value){
            collider->setRgb(Vector3(value, rgb[1], rgb[2]));
            collider->notifyUpdate(); });

        GeneralSliderView::SliderPtr slider_green = sliderView->getOrCreateSlider(_("green"), effect_range[4][0], effect_range[4][1], 2);
        slider_green->setValue(rgb[1]);
        slider_green->setCallback([collider, rgb](double value){
            collider->setRgb(Vector3(rgb[0], value, rgb[2]));
            collider->notifyUpdate(); });

        GeneralSliderView::SliderPtr slider_blue = sliderView->getOrCreateSlider(_("blue"), effect_range[5][0], effect_range[5][1], 2);
        slider_blue->setValue(rgb[2]);
        slider_blue->setCallback([collider, rgb](double value){
            collider->setRgb(Vector3(rgb[0], rgb[1], value));
            collider->notifyUpdate(); });

        GeneralSliderView::SliderPtr slider_coefb = sliderView->getOrCreateSlider(_("coef B"), effect_range[6][0], effect_range[6][1], 2);
        slider_coefb->setValue(collider->coefB());
        slider_coefb->setCallback([collider](double value){
            collider->setCoefB(value);
            collider->notifyUpdate(); });

        GeneralSliderView::SliderPtr slider_coefd = sliderView->getOrCreateSlider(_("coef D"), effect_range[7][0], effect_range[7][1], 2);
        slider_coefd->setValue(collider->coefD());
        slider_coefd->setCallback([collider](double value){
            collider->setCoefD(value);
            collider->notifyUpdate(); });

        GeneralSliderView::SliderPtr slider_stddev = sliderView->getOrCreateSlider(_("std dev"), effect_range[8][0], effect_range[8][1], 2);
        slider_stddev->setValue(collider->stdDev());
        slider_stddev->setCallback([collider](double value){
            collider->setStdDev(value);
            collider->notifyUpdate(); });

        GeneralSliderView::SliderPtr slider_salt_amount = sliderView->getOrCreateSlider(_("salt amount"), effect_range[9][0], effect_range[9][1], 2);
        slider_salt_amount->setValue(collider->saltAmount());
        slider_salt_amount->setCallback([collider](double value){
            collider->setSaltAmount(value);
            collider->notifyUpdate(); });

        GeneralSliderView::SliderPtr slider_salt_chance = sliderView->getOrCreateSlider(_("salt chance"), 0.0, 1.0, 2);
        slider_salt_chance->setValue(collider->saltChance());
        slider_salt_chance->setCallback([collider](double value){
            collider->setSaltChance(value);
            collider->notifyUpdate(); });

        GeneralSliderView::SliderPtr slider_pepper_amount = sliderView->getOrCreateSlider(_("pepper amount"), effect_range[10][0], effect_range[10][1], 2);
        slider_pepper_amount->setValue(collider->pepperAmount());
        slider_pepper_amount->setCallback([collider](double value){
            collider->setPepperAmount(value);
            collider->notifyUpdate(); });

        GeneralSliderView::SliderPtr slider_pepper_chance = sliderView->getOrCreateSlider(_("pepper chance"), 0.0, 1.0, 2);
        slider_pepper_chance->setValue(collider->pepperChance());
        slider_pepper_chance->setCallback([collider](double value){
            collider->setPepperChance(value);
            collider->notifyUpdate(); });

        GeneralSliderView::SliderPtr slider_mosaic_chance = sliderView->getOrCreateSlider(_("mosaic chance"), 0.0, 1.0, 2);
        slider_mosaic_chance->setValue(collider->mosaicChance());
        slider_mosaic_chance->setCallback([collider](double value){
            collider->setMosaicChance(value);
            collider->notifyUpdate(); });

        GeneralSliderView::SliderPtr slider_kernel = sliderView->getOrCreateSlider(_("kernel"), 8.0, 64.0, 2);
        slider_kernel->setValue(collider->kernel());
        slider_kernel->setCallback([collider](double value){
            collider->setKernel(value);
            collider->notifyUpdate(); });

        sliders.push_back(slider_hue);
        sliders.push_back(slider_saturation);
        sliders.push_back(slider_value);
        sliders.push_back(slider_red);
        sliders.push_back(slider_green);
        sliders.push_back(slider_blue);
        sliders.push_back(slider_coefb);
        sliders.push_back(slider_coefd);
        sliders.push_back(slider_stddev);
        sliders.push_back(slider_salt_amount);
        sliders.push_back(slider_salt_chance);
        sliders.push_back(slider_pepper_amount);
        sliders.push_back(slider_pepper_chance);
        sliders.push_back(slider_mosaic_chance);
        sliders.push_back(slider_kernel);
    }
}

class MultiColliderItemCreationPanel : public ItemCreationPanelBase<MultiColliderItem>
{
    QLineEdit* nameEntry;
    QComboBox* colliderCombo;

public:
    MultiColliderItemCreationPanel()
    {
        auto vbox = new QVBoxLayout;
        setLayout(vbox);

        auto hbox1 = new QHBoxLayout;
        hbox1->addWidget(new QLabel(_("Name :")));
        nameEntry = new QLineEdit;
        hbox1->addWidget(nameEntry);
        vbox->addLayout(hbox1);

        auto hbox2 = new QHBoxLayout;

        hbox2->addWidget(new QLabel(_("Type :")));
        colliderCombo = new QComboBox;
        colliderCombo->addItem(_("CFD"));
        colliderCombo->addItem(_("TC"));
        colliderCombo->addItem(_("VFX"));
        connect(colliderCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            [=](int index){ onColliderComboCurrentIndexChanged(index); });
        hbox2->addWidget(colliderCombo);

        vbox->addLayout(hbox2);
    }

    virtual bool initializeCreation(MultiColliderItem* protoItem, Item* parentItem) override
    {
        nameEntry->setText(protoItem->name().c_str());
        nameEntry->setText("CFDCollider");
        return true;
    }

    virtual bool updateItem(MultiColliderItem* protoItem, Item* parentItem) override
    {
        protoItem->setName(nameEntry->text().toStdString());
        protoItem->setColliderType(colliderCombo->currentIndex());

        return true;
    }

    void onColliderComboCurrentIndexChanged(int index)
    {
        const QStringList texts = { "CFD", "TC", "VFX" };
        QString text = texts.at(index) + "Collider";
        nameEntry->setText(text);
    }
};

}


void MultiColliderItem::initializeClass(ExtensionManager* ext)
{
    ext->itemManager()
        .registerClass<MultiColliderItem, SimpleColliderItem>(N_("MultiColliderItem"))
        // .addCreationPanel<MultiColliderItem>();
        .addCreationPanel<MultiColliderItem>(new MultiColliderItemCreationPanel)
        .addAlias<MultiColliderItem>("FluidAreaItem", "CFD");

    auto rootItem = RootItem::instance();
    rootItem->sigSelectedItemsChanged().connect(
        [&](const ItemList<>& selectedItems){ onSelectedItemChanged(selectedItems); });
}