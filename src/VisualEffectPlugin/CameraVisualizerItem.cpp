/**
   \file
   \author Kenta Suzuki
*/

#include "CameraVisualizerItem.h"
#include <cnoid/Archive>
#include <cnoid/Body>
#include <cnoid/BodyItem>
#include <cnoid/ConnectionSet>
#include <cnoid/ImageView>
#include <cnoid/ImageableItem>
#include <cnoid/ItemManager>
#include <cnoid/PutPropertyFunction>
#include <cnoid/RangeCamera>
#include <cnoid/SpotLight>
#include "gettext.h"
#include "VisualEffectDialog.h"
#include "VisualEffect.h"
#include "ImageGenerator.h"

using namespace std;
using namespace cnoid;

namespace {

class CameraImageVisualizerItem2;
CameraImageVisualizerItem2* pitem = nullptr;
VisualEffectDialog* effectDialog = nullptr;

class CameraVisualizerItemBase
{
public:
    Item* visualizerItem;
    BodyItem* bodyItem;
    ScopedConnection sigCheckToggledConnection;
    CameraVisualizerItemBase(Item* visualizerItem);
    void setBodyItem(BodyItem* bodyItem);
    void updateVisualization();

    virtual void enableVisualization(bool on) = 0;
    virtual void doUpdateVisualization() = 0;
};


class CameraImageVisualizerItem2 : public Item, public ImageableItem, public CameraVisualizerItemBase
{
public:
    CameraImageVisualizerItem2();
    virtual const Image* getImage() override;
    virtual SignalProxy<void()> sigImageUpdated() override;
    void setBodyItem(BodyItem* bodyItem, Camera* camera);
    virtual void enableVisualization(bool on) override;
    virtual void doUpdateVisualization() override;

    CameraPtr camera;
    VisualEffect effect;
    ScopedConnectionSet connections;
    std::shared_ptr<const Image> image;
    Signal<void()> sigImageUpdated_;
};


class LightSwitcherItem : public Item, public CameraVisualizerItemBase
{
public:
    LightSwitcherItem();
    void setBodyItem(BodyItem* bodyItem, SpotLight* light);
    virtual void enableVisualization(bool on) override;
    virtual void doUpdateVisualization() override;
    void updateLightState();

protected:
    virtual void doPutProperties(PutPropertyFunction& putProperty) override;

    SpotLightPtr light;
    ScopedConnectionSet connections;
};

}

namespace cnoid {

class CameraVisualizerItemImpl
{
public:
    CameraVisualizerItem* self;
    BodyItem* bodyItem;
    vector<Item*> subItems;
    vector<ItemPtr> restoredSubItems;

    CameraVisualizerItemImpl(CameraVisualizerItem* self);
    void onPositionChanged();
};

}


void CameraVisualizerItem::initializeClass(ExtensionManager* ext)
{
    ItemManager& im = ext->itemManager();
    im.registerClass<CameraVisualizerItem>(N_("CameraVisualizer"));
    im.addCreationPanel<CameraVisualizerItem>();

    im.registerClass<CameraImageVisualizerItem2>(N_("CameraImageVisualizer2"));
    im.registerClass<LightSwitcherItem>(N_("LightSwitcher"));
}


CameraVisualizerItem::CameraVisualizerItem()
{
    impl = new CameraVisualizerItemImpl(this);
}


CameraVisualizerItemImpl::CameraVisualizerItemImpl(CameraVisualizerItem* self)
    : self(self)
{

}


CameraVisualizerItem::CameraVisualizerItem(const CameraVisualizerItem& org)
    : Item(org)
{
    impl = new CameraVisualizerItemImpl(this);
}


CameraVisualizerItem::~CameraVisualizerItem()
{
    delete impl;
}


Item* CameraVisualizerItem::doDuplicate() const
{
    return new CameraVisualizerItem(*this);
}


void CameraVisualizerItem::onPositionChanged()
{
    if(parentItem()){
        impl->onPositionChanged();
    }
}


void CameraVisualizerItemImpl::onPositionChanged()
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
                    CameraImageVisualizerItem2* cameraImageVisualizerItem =
                            j<n ? dynamic_cast<CameraImageVisualizerItem2*>(restoredSubItems[j++].get()) : new CameraImageVisualizerItem2();
                    if(cameraImageVisualizerItem){
                        cameraImageVisualizerItem->setBodyItem(bodyItem, cameras[i]);
                        self->addSubItem(cameraImageVisualizerItem);
                        subItems.push_back(cameraImageVisualizerItem);
                    }
                }
            }

            DeviceList<SpotLight> lights = body->devices<SpotLight>();
            for(size_t i=0; i < lights.size(); ++i){
                LightSwitcherItem* lightStateSwitcherItem =
                        j<n ? dynamic_cast<LightSwitcherItem*>(restoredSubItems[j++].get()) : new LightSwitcherItem();
                if(lightStateSwitcherItem){
                    lightStateSwitcherItem->setBodyItem(bodyItem, lights[i]);
                    self->addSubItem(lightStateSwitcherItem);
                    subItems.push_back(lightStateSwitcherItem);
                }
            }
        }

        restoredSubItems.clear();
    }
}

void CameraVisualizerItem::onDisconnectedFromRoot()
{
    for(size_t i=0; i < impl->subItems.size(); i++){
        impl->subItems[i]->removeFromParentItem();
    }
    impl->subItems.clear();
}


bool CameraVisualizerItem::store(Archive& archive)
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
        CameraImageVisualizerItem2* vitem = dynamic_cast<CameraImageVisualizerItem2*>(item);
        if(vitem) {
            subArchive->write("hue", vitem->effect.hue());
            subArchive->write("saturation", vitem->effect.saturation());
            subArchive->write("value", vitem->effect.value());
            subArchive->write("red", vitem->effect.red());
            subArchive->write("green", vitem->effect.green());
            subArchive->write("blue", vitem->effect.blue());
            subArchive->write("coef_b", vitem->effect.coefB());
            subArchive->write("coef_d", vitem->effect.coefD());
            subArchive->write("std_dev", vitem->effect.stdDev());
            subArchive->write("salt", vitem->effect.salt());
            subArchive->write("pepper", vitem->effect.pepper());
            subArchive->write("flip", vitem->effect.flip());
            subArchive->write("filter", vitem->effect.filter());
        }
        item->store(*subArchive);

        subItems->append(subArchive);
    }

    archive.insert("sub_items", subItems);

    return true;
}


bool CameraVisualizerItem::restore(const Archive& archive)
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
                Item* tmpItem = item;
                if(tmpItem) {
                    CameraImageVisualizerItem2* vitem = dynamic_cast<CameraImageVisualizerItem2*>(tmpItem);
                    if(vitem) {
                        double value;
                        subArchive->read("hue", value);
                        if(fabs(value) < 0.01) {
                            value = 0.0;
                        }
                        vitem->effect.setHue(value);
                        if(fabs(value) < 0.01) {
                            value = 0.0;
                        }
                        subArchive->read("saturation", value);
                        if(fabs(value) < 0.01) {
                            value = 0.0;
                        }
                        vitem->effect.setSaturation(value);
                        subArchive->read("value", value);
                        if(fabs(value) < 0.01) {
                            value = 0.0;
                        }
                        vitem->effect.setValue(value);
                        subArchive->read("red", value);
                        if(fabs(value) < 0.01) {
                            value = 0.0;
                        }
                        vitem->effect.setRed(value);
                        subArchive->read("green", value);
                        if(fabs(value) < 0.01) {
                            value = 0.0;
                        }
                        vitem->effect.setGreen(value);
                        subArchive->read("blue", value);
                        if(fabs(value) < 0.01) {
                            value = 0.0;
                        }
                        vitem->effect.setBlue(value);
                        subArchive->read("coef_b", value);
                        if(fabs(value) < 0.01) {
                            value = 0.0;
                        }
                        vitem->effect.setCoefB(value);
                        subArchive->read("coef_d", value);
                        if(fabs(value) < 1.0) {
                            value = 1.0;
                        }
                        vitem->effect.setCoefD(value);
                        subArchive->read("std_dev", value);
                        if(fabs(value) < 0.01) {
                            value = 0.0;
                        }
                        vitem->effect.setStdDev(value);
                        subArchive->read("salt", value);
                        if(fabs(value) < 0.01) {
                            value = 0.0;
                        }
                        vitem->effect.setSalt(value);
                        subArchive->read("pepper", value);
                        if(fabs(value) < 0.01) {
                            value = 0.0;
                        }
                        vitem->effect.setPepper(value);
                        bool flip;
                        subArchive->read("flip", flip);
                        vitem->effect.setFlip(flip);
                        int filter = 0;
                        subArchive->read("filter", filter);
                        vitem->effect.setFilter(filter);
                    }
                }
                impl->restoredSubItems.push_back(item);
            }
        }
    }
    return true;
}


CameraVisualizerItemBase::CameraVisualizerItemBase(Item* visualizerItem)
    : visualizerItem(visualizerItem),
      bodyItem(nullptr)
{
    sigCheckToggledConnection.reset(
        visualizerItem->sigCheckToggled(Item::LogicalSumOfAllChecks).connect(
            [&](bool on){ enableVisualization(on); }));
}


void CameraVisualizerItemBase::setBodyItem(BodyItem* bodyItem)
{
    this->bodyItem = bodyItem;
    enableVisualization(visualizerItem->isChecked(Item::LogicalSumOfAllChecks));
}


void CameraVisualizerItemBase::updateVisualization()
{
    if(visualizerItem->isChecked(Item::LogicalSumOfAllChecks)){
        doUpdateVisualization();
    }
}


CameraImageVisualizerItem2::CameraImageVisualizerItem2()
    : CameraVisualizerItemBase(this)
{
    effectDialog = VisualEffectDialog::instance();
}


const Image* CameraImageVisualizerItem2::getImage()
{
    return image.get();
}


SignalProxy<void()> CameraImageVisualizerItem2::sigImageUpdated()
{
    return sigImageUpdated_;
}


void CameraImageVisualizerItem2::setBodyItem(BodyItem* bodyItem, Camera* camera)
{
    if(name().empty()){
        string name = camera->name();
        if(dynamic_cast<RangeCamera*>(camera))
            name += "_Image";
        setName(name);
    }

    this->camera = camera;

    CameraVisualizerItemBase::setBodyItem(bodyItem);
}


void CameraImageVisualizerItem2::enableVisualization(bool on)
{
    connections.disconnect();

    if(camera && on){
        connections.add(
            camera->sigStateChanged().connect(
                [&](){ doUpdateVisualization(); }));

        doUpdateVisualization();
    }
}


void CameraImageVisualizerItem2::doUpdateVisualization()
{
    if(camera){
        ImageableItem* item = ImageViewBar::instance()->getSelectedImageableItem();
        CameraImageVisualizerItem2* eitem = dynamic_cast<CameraImageVisualizerItem2*>(item);

        if(eitem) {
            if(eitem == this) {
                if(pitem != this) {
                    effectDialog->setVisualEffect(effect);
                }
                effect.setHue(effectDialog->hue());
                effect.setSaturation(effectDialog->saturation());
                effect.setValue(effectDialog->value());
                effect.setRed(effectDialog->red());
                effect.setGreen(effectDialog->green());
                effect.setBlue(effectDialog->blue());
                effect.setCoefB(effectDialog->coefB());
                effect.setCoefD(effectDialog->coefD());
                effect.setStdDev(effectDialog->stdDev());
                effect.setSalt(effectDialog->salt());
                effect.setPepper(effectDialog->pepper());
                effect.setFlip(effectDialog->flip());
                effect.setFilter(effectDialog->filter());
                pitem = this;
            }
        }

        ImageGenerator generator;
        Image orgImage = *camera->sharedImage();

        double hue = effect.hue();
        double saturation = effect.saturation();
        double value = effect.value();
        double red = effect.red();
        double green = effect.green();
        double blue = effect.blue();
        bool flipped = effect.flip();
        double coefB = effect.coefB();
        double coefD = effect.coefD();
        double stdDev = effect.stdDev();
        double salt = effect.salt();
        double pepper = effect.pepper();
        int filter = effect.filter();

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


LightSwitcherItem::LightSwitcherItem()
    : CameraVisualizerItemBase(this)
{

}


void LightSwitcherItem::setBodyItem(BodyItem* bodyItem, SpotLight* light)
{
    if(name().empty()){
        string name = light->name();
        if(name.empty()) {
            name = "NoName";
        }
        setName(name);
    }

    this->light = light;

    CameraVisualizerItemBase::setBodyItem(bodyItem);
}


void LightSwitcherItem::enableVisualization(bool on)
{
    connections.disconnect();

    if(light && on){
        connections.add(
            light->sigStateChanged().connect(
                [&](){ updateLightState(); }));

        doUpdateVisualization();
    }

    if(light) {
        light->on(on);
        light->notifyStateChange();
    }
}


void LightSwitcherItem::doUpdateVisualization()
{

}


void LightSwitcherItem::doPutProperties(PutPropertyFunction& putProperty)
{

}


void LightSwitcherItem::updateLightState()
{

}
