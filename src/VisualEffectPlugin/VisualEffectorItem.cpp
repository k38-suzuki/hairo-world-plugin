/**
   \file
   \author Kenta Suzuki
*/

#include "VisualEffectorItem.h"
#include <cnoid/Archive>
#include <cnoid/Body>
#include <cnoid/ItemManager>
#include <cnoid/ItemTreeView>
#include <cnoid/MenuManager>
#include <cnoid/PutPropertyFunction>
#include <cnoid/RootItem>
#include "gettext.h"
#include "VEAreaItem.h"

using namespace cnoid;
using namespace std;

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
                [item](){ item->effector->show(); });
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
        VEImageVisualizerItem* vitem = dynamic_cast<VEImageVisualizerItem*>(item);
        VisualEffector* effector = vitem->effector;
        effector->store(*subArchive);
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
                Item* tmpItem = item;
                if(tmpItem) {
                    VEImageVisualizerItem* vitem = dynamic_cast<VEImageVisualizerItem*>(tmpItem);
                    VisualEffector* effector = vitem->effector;
                    effector->restore(*subArchive);
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
    effector = new VisualEffector();
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
        double hue = effector->hue();
        double saturation = effector->saturation();
        double value = effector->value();
        double red = effector->red();
        double green = effector->green();
        double blue = effector->blue();
        bool flipped = effector->flip();
        double coefB = effector->coefB();
        double coefD = effector->coefD();
        double stdDev = effector->stdDev();
        double salt = effector->salt();
        double pepper = effector->pepper();
        int filter = effector->filter();

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
