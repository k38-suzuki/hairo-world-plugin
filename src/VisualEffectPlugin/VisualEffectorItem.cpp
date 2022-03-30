/**
   \file
   \author Kenta Suzuki
*/

#include "VisualEffectorItem.h"
#include <cnoid/Action>
#include <cnoid/Archive>
#include <cnoid/Body>
#include <cnoid/BodyItem>
#include <cnoid/Button>
#include <cnoid/CheckBox>
#include <cnoid/ComboBox>
#include <cnoid/ConnectionSet>
#include <cnoid/Dialog>
#include <cnoid/ImageableItem>
#include <cnoid/ImageView>
#include <cnoid/ItemManager>
#include <cnoid/ItemTreeView>
#include <cnoid/Menu>
#include <cnoid/MenuManager>
#include <cnoid/PutPropertyFunction>
#include <cnoid/RangeCamera>
#include <cnoid/Separator>
#include <cnoid/SimulationBar>
#include <cnoid/SimulatorItem>
#include <cnoid/SpinBox>
#include <cnoid/ViewManager>
#include <cnoid/WorldItem>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <math.h>
#include <mutex>
#include "ImageGenerator.h"
#include "VEAreaItem.h"
#include "gettext.h"

using namespace cnoid;
using namespace std;

namespace {

Menu contextMenu;

enum DoubleSpinID {
    HUE, SATURATION, VALUE,
    RED, GREEN, BLUE,
    COEFB, COEFD, STDDEV,
    SALT, PEPPER, NUM_DSPINS
};

struct WidgetInfo {
    int row;
    int column;
};

WidgetInfo dspinInfo[] = {
    { 0, 1 }, { 0, 3 }, { 0, 5 },
    { 1, 1 }, { 1, 3 }, { 1, 5 },
    { 2, 1 }, { 2, 3 },
    { 3, 1 }, { 3, 3 }, { 3, 5 }
};

WidgetInfo labelInfo[] = {
    { 0, 0 }, { 0, 2 }, { 0, 4 },
    { 1, 0 }, { 1, 2 }, { 1, 4 },
    { 2, 0 }, { 2, 2 },
    { 3, 0 }, { 3, 2 }, { 3, 4 }
};

class EffectConfigDialog : public Dialog
{
public:
    EffectConfigDialog();

    DoubleSpinBox* dspins[NUM_DSPINS];
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
    std::mutex mtx;
    SimulatorItem* simulatorItem;
    ImageGenerator generator;
    ScopedConnectionSet connections;
    std::shared_ptr<const Image> image;
    Signal<void()> sigImageUpdated_;

protected:
    virtual bool store(Archive& archive) override;
    virtual bool restore(const Archive& archive) override;
};

void onViewCreated(View* view)
{
    ImageView* imageView = dynamic_cast<ImageView*>(view);
    if(imageView) {
        imageView->setContextMenuPolicy(Qt::CustomContextMenu);
        QObject::connect(imageView, &ImageView::customContextMenuRequested,
                [=](const QPoint& pos){ contextMenu.exec( imageView->mapToGlobal(pos)); });
    }
}

void onShowConfigTriggered()
{
    ImageableItem* imageableItem = ImageViewBar::instance()->getSelectedImageableItem();
    if(imageableItem) {
        VEImageVisualizerItem* visualizerItem = dynamic_cast<VEImageVisualizerItem*>(imageableItem);
        if(visualizerItem) {
            visualizerItem->config->show();
        }
    }
}

}

namespace cnoid {

class VisualEffectorItemImpl
{
public:
    VisualEffectorItemImpl(VisualEffectorItem* self);
    VisualEffectorItemImpl(VisualEffectorItem* self, const VisualEffectorItemImpl& org);

    VisualEffectorItem* self;
    BodyItem* bodyItem;
    vector<Item*> subItems;
    vector<ItemPtr> restoredSubItems;

    void onPositionChanged();
};

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
    : Item(org),
      impl(new VisualEffectorItemImpl(this, *org.impl))
{

}


VisualEffectorItemImpl::VisualEffectorItemImpl(VisualEffectorItem* self, const VisualEffectorItemImpl& org)
{

}


VisualEffectorItem::~VisualEffectorItem()
{
    delete impl;
}


void VisualEffectorItem::initializeClass(ExtensionManager* ext)
{
    ext->itemManager()
            .registerClass<VisualEffectorItem>(N_("VisualEffectorItem"))
            .addCreationPanel<VisualEffectorItem>()

            .registerClass<VEImageVisualizerItem>(N_("VEImageVisualizerItem"));

    ItemTreeView::instance()->customizeContextMenu<VEImageVisualizerItem>(
        [](VEImageVisualizerItem* item, MenuManager& menuManager, ItemFunctionDispatcher menuFunction) {
            menuManager.setPath("/");
            menuManager.addItem(_("Visual Effect"))->sigTriggered().connect(
                        [item](){ item->config->show(); });
            menuManager.setPath("/");
            menuManager.addSeparator();
            menuFunction.dispatchAs<Item>(item);
        });

    Action* showConfig = new Action;
    showConfig->setText(_("Visual Effect"));
    contextMenu.addAction(showConfig);
    showConfig->sigTriggered().connect([&](){ onShowConfigTriggered(); });
    ext->viewManager().sigViewCreated().connect([&](View* view){ onViewCreated(view); });
}


Item* VisualEffectorItem::doDuplicate() const
{
    return new VisualEffectorItem(*this);
}


void VisualEffectorItem::onPositionChanged()
{
    if(parentItem()) {
        impl->onPositionChanged();
    }
}


void VisualEffectorItemImpl::onPositionChanged()
{
    BodyItem* newBodyItem = self->findOwnerItem<BodyItem>();
    if(newBodyItem != bodyItem) {
        bodyItem = newBodyItem;
        for(size_t i=0; i < subItems.size(); i++) {
            subItems[i]->removeFromParentItem();
        }
        subItems.clear();

        int n = restoredSubItems.size();
        int j = 0;

        if(bodyItem) {
            Body* body = bodyItem->body();

            DeviceList<Camera> cameras = body->devices<Camera>();
            for(size_t i = 0; i < cameras.size(); ++i) {
                if(cameras[i]->imageType() != Camera::NO_IMAGE) {
                    VEImageVisualizerItem* cameraImageVisualizerItem =
                            j < n ? dynamic_cast<VEImageVisualizerItem*>(restoredSubItems[j++].get()) : new VEImageVisualizerItem;
                    if(cameraImageVisualizerItem) {
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
    for(size_t i = 0; i < impl->subItems.size(); i++) {
        impl->subItems[i]->removeFromParentItem();
    }
    impl->subItems.clear();
}


bool VisualEffectorItem::store(Archive& archive)
{
    ListingPtr subItems = new Listing;

    for(size_t i = 0; i < impl->subItems.size(); ++i) {
        Item* item = impl->subItems[i];
        string pluginName, className;
        ItemManager::getClassIdentifier(item, pluginName, className);

        ArchivePtr subArchive = new Archive;
        subArchive->write("class", className);
        subArchive->write("name", item->name());
        if(item->isSelected()) {
            subArchive->write("is_selected", true);
        }
        if(item->isChecked()) {
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
    if(!subItems->isValid()) {
        subItems = archive.findListing("subItems"); // Old
    }
    if(subItems->isValid()) {
        for(int i = 0; i < subItems->size(); ++i) {
            Archive* subArchive = dynamic_cast<Archive*>(subItems->at(i)->toMapping());
            string className, itemName;
            subArchive->read("class", className);
            subArchive->read("name", itemName);
            if(ItemPtr item = ItemManager::createItem("VisualEffect", className)) {
                item->setName(itemName);
                item->restore(*subArchive);
                if(subArchive->get("is_selected", false)) {
                    item->setSelected(true);
                }
                if(subArchive->get("is_checked", false)) {
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
    if(visualizerItem->isChecked(Item::LogicalSumOfAllChecks)) {
        doUpdateVisualization();
    }
}


VEImageVisualizerItem::VEImageVisualizerItem()
    : VisualEffectorItemBase(this)
{
    config = new EffectConfigDialog;
    simulatorItem = nullptr;
    SimulationBar::instance()->sigSimulationAboutToStart().connect(
                [&](SimulatorItem* simulatorItem){ this->simulatorItem = simulatorItem; });
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
    if(name().empty()) {
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

    if(camera && on) {
        connections.add(
            camera->sigStateChanged().connect(
                [&]() { doUpdateVisualization(); }));

        doUpdateVisualization();
    }
}


void VEImageVisualizerItem::doUpdateVisualization()
{
    if(camera) {
        std::lock_guard<std::mutex> lock(mtx);
        double hue = config->dspins[HUE]->value();
        double saturation = config->dspins[SATURATION]->value();
        double value = config->dspins[VALUE]->value();
        double red = config->dspins[RED]->value();
        double green = config->dspins[GREEN]->value();
        double blue = config->dspins[BLUE]->value();
        bool flipped = config->flipCheck->isChecked();
        double coefB = config->dspins[COEFB]->value();
        double coefD = config->dspins[COEFD]->value();
        double stdDev = config->dspins[STDDEV]->value();
        double salt = config->dspins[SALT]->value();
        double pepper = config->dspins[PEPPER]->value();
        int filter = config->filterCombo->currentIndex();

        if(simulatorItem) {
            WorldItem* worldItem = simulatorItem->findOwnerItem<WorldItem>();
            if(worldItem) {
                ItemList<VEAreaItem> areaItems = worldItem->descendantItems<VEAreaItem>();
                for(size_t i = 0; i < areaItems.size(); ++i) {
                    VEAreaItem* areaItem = areaItems[i];
                    bool isCollided = areaItem->isCollided(camera->link()->T().translation());
                    if(isCollided) {
                        hue = areaItem->hsv()[0];
                        saturation = areaItem->hsv()[1];
                        value = areaItem->hsv()[2];
                        red = areaItem->rgb()[0];
                        green = areaItem->rgb()[1];
                        blue = areaItem->rgb()[2];
                        coefB = areaItem->coefB();
                        coefD = areaItem->coefD();
                        stdDev = areaItem->stdDev();
                        salt = areaItem->salt();
                        pepper = areaItem->pepper();
                        flipped = areaItem->flip();
                        filter = areaItem->filter();
                    }
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

    static const char* labels[] = {
        _("Hue"), _("Saturation"), _("Value"),
        _("Red"), _("Green"), _("Blue"),
        _("CoefB"), _("CoefD"),
        _("Std_dev"), _("Salt"), _("Pepper")
    };

    QGridLayout* gbox = new QGridLayout;
    for(int i = 0; i < NUM_DSPINS; ++i) {
        dspins[i] = new DoubleSpinBox;
        DoubleSpinBox* dspin = dspins[i];
        WidgetInfo dinfo = dspinInfo[i];
        WidgetInfo linfo = labelInfo[i];
        dspin->setRange(ranges[i][0], ranges[i][1]);
        dspin->setSingleStep(0.1);
        gbox->addWidget(new QLabel(labels[i]), linfo.row, linfo.column);
        gbox->addWidget(dspin, dinfo.row, dinfo.column);
    }
    dspins[COEFD]->setValue(1.0);


    flipCheck = new CheckBox;
    flipCheck->setText(_("Flip"));
    flipCheck->setChecked(false);
    gbox->addWidget(flipCheck, 4, 0);

    filterCombo = new ComboBox;
    QStringList filters = {
        _("No filter"), _("Gaussian 3x3"), _("Gaussian 5x5"),
        _("Sobel"), _("Prewitt")
    };
    filterCombo->addItems(filters);
    filterCombo->setCurrentIndex(0);
    gbox->addWidget(new QLabel(_("Filter")), 4, 2);
    gbox->addWidget(filterCombo, 4, 3);

    PushButton* resetButton = new PushButton(_("&Reset"));
    QPushButton* okButton = new QPushButton(_("&Ok"));
    okButton->setDefault(true);
    QDialogButtonBox* buttonBox = new QDialogButtonBox(this);
    buttonBox->addButton(resetButton, QDialogButtonBox::ResetRole);
    buttonBox->addButton(okButton, QDialogButtonBox::AcceptRole);
    connect(buttonBox, &QDialogButtonBox::accepted, [this](){ this->accept(); });

    QVBoxLayout* vbox = new QVBoxLayout;
    vbox->addLayout(gbox);
    vbox->addWidget(new HSeparator);
    vbox->addWidget(buttonBox);
    setLayout(vbox);

    resetButton->sigClicked().connect([&](){ onResetButtonClicked(); });
}


void EffectConfigDialog::onResetButtonClicked()
{
    for(int i = 0; i < NUM_DSPINS; ++i) {
        dspins[i]->setValue(0.0);
    }
    dspins[COEFD]->setValue(1.0);
    flipCheck->setChecked(false);
    filterCombo->setCurrentIndex(0);
}


bool EffectConfigDialog::storeState(Archive& archive)
{
    archive.write("hue", dspins[HUE]->value());
    archive.write("saturation", dspins[SATURATION]->value());
    archive.write("value", dspins[VALUE]->value());
    archive.write("red", dspins[RED]->value());
    archive.write("green", dspins[GREEN]->value());
    archive.write("blue", dspins[BLUE]->value());
    archive.write("coef_b", dspins[COEFB]->value());
    archive.write("coef_d", dspins[COEFD]->value());
    archive.write("std_dev", dspins[STDDEV]->value());
    archive.write("salt", dspins[SALT]->value());
    archive.write("pepper", dspins[PEPPER]->value());
    archive.write("flip", flipCheck->isChecked());
    archive.write("filter", filterCombo->currentIndex());
    return true;
}


bool EffectConfigDialog::restoreState(const Archive& archive)
{
    dspins[HUE]->setValue(archive.get("hue", 0.0));
    dspins[SATURATION]->setValue(archive.get("saturation", 0.0));
    dspins[VALUE]->setValue(archive.get("value", 0.0));
    dspins[RED]->setValue(archive.get("red", 0.0));
    dspins[GREEN]->setValue(archive.get("green", 0.0));
    dspins[BLUE]->setValue(archive.get("blue", 0.0));
    dspins[COEFB]->setValue(archive.get("coef_b", 0.0));
    dspins[COEFD]->setValue(archive.get("coef_d", 1.0));
    dspins[STDDEV]->setValue(archive.get("std_dev", 0.0));
    dspins[SALT]->setValue(archive.get("salt", 0.0));
    dspins[PEPPER]->setValue(archive.get("pepper", 0.0));
    flipCheck->setChecked(archive.get("flip", false));
    filterCombo->setCurrentIndex(archive.get("filter", 0));
    return true;
}
