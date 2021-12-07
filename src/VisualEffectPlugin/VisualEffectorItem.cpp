/**
   \file
   \author Kenta Suzuki
*/

#include "VisualEffectorItem.h"
#include <cnoid/Archive>
#include <cnoid/Body>
#include <cnoid/BodyItem>
#include <cnoid/Button>
#include <cnoid/CheckBox>
#include <cnoid/ComboBox>
#include <cnoid/ConnectionSet>
#include <cnoid/Dialog>
#include <cnoid/ImageableItem>
#include <cnoid/ItemManager>
#include <cnoid/ItemTreeView>
#include <cnoid/MenuManager>
#include <cnoid/PutPropertyFunction>
#include <cnoid/RootItem>
#include "gettext.h"
#include "VEAreaItem.h"
#include <cnoid/RangeCamera>
#include <cnoid/Separator>
#include <cnoid/SpinBox>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <math.h>
#include "ImageGenerator.h"

using namespace cnoid;
using namespace std;

namespace {

class EffectConfigDialog : public Dialog
{
public:
    EffectConfigDialog();

    DoubleSpinBox* hueSpin;
    DoubleSpinBox* saturationSpin;
    DoubleSpinBox* valueSpin;
    DoubleSpinBox* redSpin;
    DoubleSpinBox* greenSpin;
    DoubleSpinBox* blueSpin;
    DoubleSpinBox* coefBSpin;
    DoubleSpinBox* coefDSpin;
    DoubleSpinBox* stdDevSpin;
    DoubleSpinBox* saltSpin;
    DoubleSpinBox* pepperSpin;
    CheckBox* flipCheck;
    ComboBox* filterCombo;

    void onResetButtonClicked();
    bool storeState(Archive& archive);
    bool restoreState(const Archive& archive);
};


class VisualEffectorItemBase
{
public:
    Item* visualizerItem;
    BodyItem* bodyItem;
    ScopedConnection sigCheckToggledConnection;
    VisualEffectorItemBase(Item* visualizerItem);
    void setBodyItem(BodyItem* bodyItem);
    void updateVisualization();

    virtual void enableVisualization(bool on) = 0;
    virtual void doUpdateVisualization() = 0;
};


class VEImageVisualizerItem : public Item, public ImageableItem, public VisualEffectorItemBase
{
public:
    VEImageVisualizerItem();
    virtual const Image* getImage() override;
    virtual SignalProxy<void()> sigImageUpdated() override;
    void setBodyItem(BodyItem* bodyItem, Camera* camera);
    virtual void enableVisualization(bool on) override;
    virtual void doUpdateVisualization() override;

    CameraPtr camera;
    EffectConfigDialog* config;
    ImageGenerator generator;
    ScopedConnectionSet connections;
    std::shared_ptr<const Image> image;
    Signal<void()> sigImageUpdated_;

protected:
    virtual bool store(Archive& archive) override;
    virtual bool restore(const Archive& archive) override;
};

}


namespace cnoid {

class VisualEffectorItemImpl
{
public:
    VisualEffectorItem* self;
    BodyItem* bodyItem;
    vector<Item*> subItems;
    vector<ItemPtr> restoredSubItems;

    VisualEffectorItemImpl(VisualEffectorItem* self);
    void onPositionChanged();
};

}


void VisualEffectorItem::initializeClass(ExtensionManager* ext)
{
    ItemManager& im = ext->itemManager();
    im.registerClass<VisualEffectorItem>(N_("VisualEffectorItem"));
    im.addCreationPanel<VisualEffectorItem>();

    im.registerClass<VEImageVisualizerItem>(N_("VEImageVisualizerItem"));

    ItemTreeView::instance()->customizeContextMenu<VEImageVisualizerItem>(
        [](VEImageVisualizerItem* item, MenuManager& menuManager, ItemFunctionDispatcher menuFunction) {
            menuManager.setPath("/");
            menuManager.addItem(_("Visual Effect"))->sigTriggered().connect(
                        [item](){ item->config->show(); });
            menuManager.setPath("/");
            menuManager.addSeparator();
            menuFunction.dispatchAs<Item>(item);
        });
}


VisualEffectorItem::VisualEffectorItem()
{
    impl = new VisualEffectorItemImpl(this);
}


VisualEffectorItemImpl::VisualEffectorItemImpl(VisualEffectorItem* self)
    : self(self)
{

}


VisualEffectorItem::VisualEffectorItem(const VisualEffectorItem& org)
    : Item(org)
{
    impl = new VisualEffectorItemImpl(this);
}


VisualEffectorItem::~VisualEffectorItem()
{
    delete impl;
}


Item* VisualEffectorItem::doDuplicate() const
{
    return new VisualEffectorItem(*this);
}


void VisualEffectorItem::onPositionChanged()
{
    if(parentItem()){
        impl->onPositionChanged();
    }
}


void VisualEffectorItemImpl::onPositionChanged()
{
    BodyItem* newBodyItem = self->findOwnerItem<BodyItem>();
    if(newBodyItem != bodyItem){
        bodyItem = newBodyItem;
        for(size_t i=0; i < subItems.size(); i++){
            subItems[i]->removeFromParentItem();
        }
        subItems.clear();

        int n = restoredSubItems.size();
        int j = 0;

        if(bodyItem){
            Body* body = bodyItem->body();

            DeviceList<Camera> cameras = body->devices<Camera>();
            for(size_t i=0; i < cameras.size(); ++i){
                if(cameras[i]->imageType()!=Camera::NO_IMAGE){
                    VEImageVisualizerItem* cameraImageVisualizerItem =
                            j<n ? dynamic_cast<VEImageVisualizerItem*>(restoredSubItems[j++].get()) : new VEImageVisualizerItem();
                    if(cameraImageVisualizerItem){
                        cameraImageVisualizerItem->setBodyItem(bodyItem, cameras[i]);
                        self->addSubItem(cameraImageVisualizerItem);
                        subItems.push_back(cameraImageVisualizerItem);
                    }
                }
            }
        }

        restoredSubItems.clear();
    }
}


void VisualEffectorItem::onDisconnectedFromRoot()
{
    for(size_t i=0; i < impl->subItems.size(); i++){
        impl->subItems[i]->removeFromParentItem();
    }
    impl->subItems.clear();
}


bool VisualEffectorItem::store(Archive& archive)
{
    ListingPtr subItems = new Listing();

    for(size_t i=0; i < impl->subItems.size(); i++){
        Item* item = impl->subItems[i];
        string pluginName, className;
        ItemManager::getClassIdentifier(item, pluginName, className);

        ArchivePtr subArchive = new Archive();
        subArchive->write("class", className);
        subArchive->write("name", item->name());
        if(item->isSelected()){
            subArchive->write("is_selected", true);
        }
        if(item->isChecked()){
            subArchive->write("is_checked", true);
        }
        item->store(*subArchive);

        subItems->append(subArchive);
    }

    archive.insert("sub_items", subItems);

    return true;
}


bool VisualEffectorItem::restore(const Archive& archive)
{
    impl->restoredSubItems.clear();

    ListingPtr subItems = archive.findListing("sub_items");
    if(!subItems->isValid()){
        subItems = archive.findListing("subItems"); // Old
    }
    if(subItems->isValid()){
        for(int i=0; i < subItems->size(); i++){
            Archive* subArchive = dynamic_cast<Archive*>(subItems->at(i)->toMapping());
            string className, itemName;
            subArchive->read("class", className);
            subArchive->read("name", itemName);
            if(ItemPtr item = ItemManager::createItem("VisualEffect", className)){
                item->setName(itemName);
                item->restore(*subArchive);
                if(subArchive->get("is_selected", false)){
                    item->setSelected(true);
                }
                if(subArchive->get("is_checked", false)){
                    item->setChecked(true);
                }
                impl->restoredSubItems.push_back(item);
            }
        }
    }
    return true;
}


VisualEffectorItemBase::VisualEffectorItemBase(Item* visualizerItem)
    : visualizerItem(visualizerItem),
      bodyItem(nullptr)
{
    sigCheckToggledConnection.reset(
        visualizerItem->sigCheckToggled(Item::LogicalSumOfAllChecks).connect(
            [&](bool on){ enableVisualization(on); }));
}


void VisualEffectorItemBase::setBodyItem(BodyItem* bodyItem)
{
    this->bodyItem = bodyItem;
    enableVisualization(visualizerItem->isChecked(Item::LogicalSumOfAllChecks));
}


void VisualEffectorItemBase::updateVisualization()
{
    if(visualizerItem->isChecked(Item::LogicalSumOfAllChecks)){
        doUpdateVisualization();
    }
}


VEImageVisualizerItem::VEImageVisualizerItem()
    : VisualEffectorItemBase(this)
{
    config = new EffectConfigDialog();
}


const Image* VEImageVisualizerItem::getImage()
{
    return image.get();
}


SignalProxy<void()> VEImageVisualizerItem::sigImageUpdated()
{
    return sigImageUpdated_;
}


void VEImageVisualizerItem::setBodyItem(BodyItem* bodyItem, Camera* camera)
{
    if(name().empty()){
        string name = camera->name();
        if(dynamic_cast<RangeCamera*>(camera))
            name += "_Image";
        setName(name);
    }

    this->camera = camera;

    VisualEffectorItemBase::setBodyItem(bodyItem);
}


void VEImageVisualizerItem::enableVisualization(bool on)
{
    connections.disconnect();

    if(camera && on){
        connections.add(
            camera->sigStateChanged().connect(
                [&](){ doUpdateVisualization(); }));

        doUpdateVisualization();
    }
}


void VEImageVisualizerItem::doUpdateVisualization()
{
    if(camera){
        double hue = config->hueSpin->value();
        double saturation = config->saturationSpin->value();
        double value = config->valueSpin->value();
        double red = config->redSpin->value();
        double green = config->greenSpin->value();
        double blue = config->blueSpin->value();
        bool flipped = config->flipCheck->isChecked();
        double coefB = config->coefBSpin->value();
        double coefD = config->coefDSpin->value();
        double stdDev = config->stdDevSpin->value();
        double salt = config->saltSpin->value();
        double pepper = config->pepperSpin->value();
        int filter = config->filterCombo->currentIndex();

        RootItem* rootItem = RootItem::instance();
        if(rootItem) {
            ItemList<VEAreaItem> vitems = rootItem->checkedItems<VEAreaItem>();
            for(size_t i = 0; i < vitems.size(); ++i) {
                VEAreaItem* vitem = vitems[i];
                bool isCollided = vitem->isCollided(camera->link());
                if(isCollided) {
                    hue = vitem->hue();
                    saturation = vitem->saturation();
                    value = vitem->value();
                    red = vitem->red();
                    green = vitem->green();
                    blue = vitem->blue();
                    coefB = vitem->coefB();
                    coefD = vitem->coefD();
                    stdDev = vitem->stdDev();
                    salt = vitem->salt();
                    pepper = vitem->pepper();
                    flipped = vitem->flip();
                    filter = vitem->filter();
                }
            }
        }

        Image orgImage = *camera->sharedImage();

        if(hue > 0.0 || saturation > 0.0 || value > 0.0) {
            generator.hsv(orgImage, hue, saturation, value);
        }
        if(red > 0.0 || green > 0.0 || blue > 0.0) {
            generator.rgb(orgImage, red, green, blue);
        }
        if(flipped) {
            generator.flippedImage(orgImage);
        }
        if(coefB < 0.0 || coefD > 1.0) {
            generator.barrelDistortion(orgImage, coefB, coefD);
        }
        if(stdDev > 0.0) {
            generator.gaussianNoise(orgImage, stdDev);
        }
        if(salt > 0.0 || pepper > 0.0) {
            generator.saltPepperNoise(orgImage, salt, pepper);
        }
        if(filter == 1) {
            generator.gaussianFilter(orgImage, 3);
        } else if(filter == 2) {
            generator.gaussianFilter(orgImage, 5);
        } else if(filter == 3) {
            generator.sobelFilter(orgImage);
        } else if(filter == 4) {
            generator.prewittFilter(orgImage);
        }

        image = make_shared<Image>(orgImage);
    } else {
        image.reset();
    }
    sigImageUpdated_();
}


bool VEImageVisualizerItem::store(Archive& archive)
{
    config->storeState(archive);
    return true;
}


bool VEImageVisualizerItem::restore(const Archive& archive)
{
    config->restoreState(archive);
    return true;
}


EffectConfigDialog::EffectConfigDialog()
{
    setWindowTitle(_("Effect Config"));

    static const double ranges[11][2] = {
        {  0.0, 1.0 }, { -1.0,  1.0 }, { -1.0, 1.0 },
        { -1.0, 1.0 }, { -1.0,  1.0 }, { -1.0, 1.0 },
        { -1.0, 0.0 }, {  1.0, 32.0 },
        {  0.0, 1.0 }, {  0.0,  1.0 }, {  0.0, 1.0 }
    };

    hueSpin = new DoubleSpinBox();
    saturationSpin = new DoubleSpinBox();
    valueSpin = new DoubleSpinBox();
    redSpin = new DoubleSpinBox();
    greenSpin = new DoubleSpinBox();
    blueSpin = new DoubleSpinBox();
    coefBSpin = new DoubleSpinBox();
    coefDSpin = new DoubleSpinBox();
    stdDevSpin = new DoubleSpinBox();
    saltSpin = new DoubleSpinBox();
    pepperSpin = new DoubleSpinBox();
    flipCheck = new CheckBox();
    filterCombo = new ComboBox();

    vector<DoubleSpinBox*> dspins;
    dspins.push_back(hueSpin);
    dspins.push_back(saturationSpin);
    dspins.push_back(valueSpin);
    dspins.push_back(redSpin);
    dspins.push_back(greenSpin);
    dspins.push_back(blueSpin);
    dspins.push_back(coefBSpin);
    dspins.push_back(coefDSpin);
    dspins.push_back(stdDevSpin);
    dspins.push_back(saltSpin);
    dspins.push_back(pepperSpin);

    for(size_t i = 0; i < dspins.size(); ++i) {
        DoubleSpinBox* dspin = dspins[i];
        dspin->setRange(ranges[i][0], ranges[i][1]);
        dspin->setSingleStep(0.1);
    }
    coefDSpin->setValue(1.0);
    flipCheck->setText(_("Flip"));
    flipCheck->setChecked(false);
    QStringList filters = { _("No filter"), _("Gaussian 3x3"), _("Gaussian 5x5"), _("Sobel"), _("Prewitt") };
    filterCombo->addItems(filters);
    filterCombo->setCurrentIndex(0);

    QGridLayout* gbox = new QGridLayout();
    int index = 0;
    gbox->addWidget(new QLabel(_("Hue")), index, 0);
    gbox->addWidget(hueSpin, index, 1);
    gbox->addWidget(new QLabel(_("Saturation")), index, 2);
    gbox->addWidget(saturationSpin, index, 3);
    gbox->addWidget(new QLabel(_("Value")), index, 4);
    gbox->addWidget(valueSpin, index++, 5);
    gbox->addWidget(new QLabel(_("Red")), index, 0);
    gbox->addWidget(redSpin, index, 1);
    gbox->addWidget(new QLabel(_("Green")), index, 2);
    gbox->addWidget(greenSpin, index, 3);
    gbox->addWidget(new QLabel(_("Blue")), index, 4);
    gbox->addWidget(blueSpin, index++, 5);
    gbox->addWidget(new QLabel(_("CoefB")), index, 0);
    gbox->addWidget(coefBSpin, index, 1);
    gbox->addWidget(new QLabel(_("CoefD")), index, 2);
    gbox->addWidget(coefDSpin, index++, 3);
    gbox->addWidget(new QLabel(_("Std_dev")), index, 0);
    gbox->addWidget(stdDevSpin, index, 1);
    gbox->addWidget(new QLabel(_("Salt")), index, 2);
    gbox->addWidget(saltSpin, index, 3);
    gbox->addWidget(new QLabel(_("Pepper")), index, 4);
    gbox->addWidget(pepperSpin, index++, 5);
    gbox->addWidget(flipCheck, index, 0);
    gbox->addWidget(new QLabel(_("Filter")), index, 2);
    gbox->addWidget(filterCombo, index++, 3);

    PushButton* resetButton = new PushButton(_("&Reset"));
    QPushButton* okButton = new QPushButton(_("&Ok"));
    okButton->setDefault(true);
    QDialogButtonBox* buttonBox = new QDialogButtonBox(this);
    buttonBox->addButton(resetButton, QDialogButtonBox::ResetRole);
    buttonBox->addButton(okButton, QDialogButtonBox::AcceptRole);
    connect(buttonBox,SIGNAL(accepted()), this, SLOT(accept()));

    QVBoxLayout* vbox = new QVBoxLayout();
    vbox->addLayout(gbox);
    vbox->addWidget(new HSeparator());
    vbox->addWidget(buttonBox);
    setLayout(vbox);

    resetButton->sigClicked().connect([&](){ onResetButtonClicked(); });
}


void EffectConfigDialog::onResetButtonClicked()
{
    hueSpin->setValue(0.0);
    saturationSpin->setValue(0.0);
    valueSpin->setValue(0.0);
    redSpin->setValue(0.0);
    greenSpin->setValue(0.0);
    blueSpin->setValue(0.0);
    coefBSpin->setValue(0.0);
    coefDSpin->setValue(1.0);
    stdDevSpin->setValue(0.0);
    saltSpin->setValue(0.0);
    pepperSpin->setValue(0.0);
    flipCheck->setChecked(false);
    filterCombo->setCurrentIndex(0);
}


bool EffectConfigDialog::storeState(Archive& archive)
{
    archive.write("hue", hueSpin->value());
    archive.write("saturation", saturationSpin->value());
    archive.write("value", valueSpin->value());
    archive.write("red", redSpin->value());
    archive.write("green", greenSpin->value());
    archive.write("blue", blueSpin->value());
    archive.write("coef_b", coefBSpin->value());
    archive.write("coef_d", coefDSpin->value());
    archive.write("std_dev", stdDevSpin->value());
    archive.write("salt", saltSpin->value());
    archive.write("pepper", pepperSpin->value());
    archive.write("flip", flipCheck->isChecked());
    archive.write("filter", filterCombo->currentIndex());
    return true;
}


bool EffectConfigDialog::restoreState(const Archive& archive)
{
    double value;
    archive.read("hue", value);
    if(fabs(value) < 0.01) {
        value = 0.0;
    }
    hueSpin->setValue(value);
    if(fabs(value) < 0.01) {
        value = 0.0;
    }
    archive.read("saturation", value);
    if(fabs(value) < 0.01) {
        value = 0.0;
    }
    saturationSpin->setValue(value);
    archive.read("value", value);
    if(fabs(value) < 0.01) {
        value = 0.0;
    }
    valueSpin->setValue(value);
    archive.read("red", value);
    if(fabs(value) < 0.01) {
        value = 0.0;
    }
    redSpin->setValue(value);
    archive.read("green", value);
    if(fabs(value) < 0.01) {
        value = 0.0;
    }
    greenSpin->setValue(value);
    archive.read("blue", value);
    if(fabs(value) < 0.01) {
        value = 0.0;
    }
    blueSpin->setValue(value);
    archive.read("coef_b", value);
    if(fabs(value) < 0.01) {
        value = 0.0;
    }
    coefBSpin->setValue(value);
    archive.read("coef_d", value);
    if(fabs(value) < 1.0) {
        value = 1.0;
    }
    coefDSpin->setValue(value);
    archive.read("std_dev", value);
    if(fabs(value) < 0.01) {
        value = 0.0;
    }
    stdDevSpin->setValue(value);
    archive.read("salt", value);
    if(fabs(value) < 0.01) {
        value = 0.0;
    }
    saltSpin->setValue(value);
    archive.read("pepper", value);
    if(fabs(value) < 0.01) {
        value = 0.0;
    }
    pepperSpin->setValue(value);
    bool flip;
    archive.read("flip", flip);
    flipCheck->setChecked(flip);
    int filter = 0;
    archive.read("filter", filter);
    filterCombo->setCurrentIndex(filter);
    return true;
}
