/**
   @author Kenta Suzuki
*/

#include "VisualEffectorItem.h"
#include <cnoid/Action>
#include <cnoid/Archive>
#include <cnoid/BasicSensors>
#include <cnoid/BodyItem>
#include <cnoid/Button>
#include <cnoid/CheckBox>
#include <cnoid/ComboBox>
#include <cnoid/ConnectionSet>
#include <cnoid/Dialog>
#include <cnoid/EigenArchive>
#include <cnoid/EigenUtil>
#include <cnoid/GeneralSliderView>
#include <cnoid/ItemManager>
#include <cnoid/ItemTreeView>
#include <cnoid/ImageView>
#include <cnoid/ImageableItem>
#include <cnoid/MeshGenerator>
#include <cnoid/PutPropertyFunction>
#include <cnoid/RenderableItem>
#include <cnoid/RangeCamera>
#include <cnoid/RootItem>
#include <cnoid/SensorVisualizerItem>
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
#include <mutex>
#include "ImageGenerator.h"
#include "VEAreaItem.h"
#include "gettext.h"

using namespace std;
using namespace cnoid;

namespace {

Menu contextMenu;
vector<GeneralSliderView::SliderPtr> sliders;

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

static const double value_range[11][2] = {
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

        GeneralSliderView::SliderPtr slider_hue = sliderView->getOrCreateSlider(_("Hue"), value_range[0][0], value_range[0][1], 2);
        slider_hue->setValue(hsv[0]);
        slider_hue->setCallback([=](double value){
            areaItem->setHsv(Vector3(value, hsv[1], hsv[2]));
            areaItem->notifyUpdate(); });

        GeneralSliderView::SliderPtr slider_saturation = sliderView->getOrCreateSlider(_("Saturation"), value_range[1][0], value_range[1][1], 2);
        slider_saturation->setValue(hsv[1]);
        slider_saturation->setCallback([=](double value){
            areaItem->setHsv(Vector3(hsv[0], value, hsv[2]));
            areaItem->notifyUpdate(); });

        GeneralSliderView::SliderPtr slider_value = sliderView->getOrCreateSlider(_("Value"), value_range[2][0], value_range[2][1], 2);
        slider_value->setValue(hsv[2]);
        slider_value->setCallback([=](double value){
            areaItem->setHsv(Vector3(hsv[0], hsv[1], value));
            areaItem->notifyUpdate(); });

        GeneralSliderView::SliderPtr slider_red = sliderView->getOrCreateSlider(_("Red"), value_range[3][0], value_range[3][1], 2);
        slider_red->setValue(rgb[0]);
        slider_red->setCallback([=](double value){
            areaItem->setRgb(Vector3(value, rgb[1], rgb[2]));
            areaItem->notifyUpdate(); });

        GeneralSliderView::SliderPtr slider_green = sliderView->getOrCreateSlider(_("Green"), value_range[4][0], value_range[4][1], 2);
        slider_green->setValue(rgb[1]);
        slider_green->setCallback([=](double value){
            areaItem->setRgb(Vector3(rgb[0], value, rgb[2]));
            areaItem->notifyUpdate(); });

        GeneralSliderView::SliderPtr slider_blue = sliderView->getOrCreateSlider(_("Blue"), value_range[5][0], value_range[5][1], 2);
        slider_blue->setValue(rgb[2]);
        slider_blue->setCallback([=](double value){
            areaItem->setRgb(Vector3(rgb[0], rgb[1], value));
            areaItem->notifyUpdate(); });

        GeneralSliderView::SliderPtr slider_coefb = sliderView->getOrCreateSlider(_("CoefB"), value_range[6][0], value_range[6][1], 2);
        slider_coefb->setValue(areaItem->coefB());
        slider_coefb->setCallback([=](double value){
            areaItem->setCoefB(value);
            areaItem->notifyUpdate(); });

        GeneralSliderView::SliderPtr slider_coefd = sliderView->getOrCreateSlider(_("CoefD"), value_range[7][0], value_range[7][1], 2);
        slider_coefd->setValue(areaItem->coefD());
        slider_coefd->setCallback([=](double value){
            areaItem->setCoefD(value);
            areaItem->notifyUpdate(); });

        GeneralSliderView::SliderPtr slider_stddev = sliderView->getOrCreateSlider(_("Std_dev"), value_range[8][0], value_range[8][1], 2);
        slider_stddev->setValue(areaItem->stdDev());
        slider_stddev->setCallback([=](double value){
            areaItem->setStdDev(value);
            areaItem->notifyUpdate(); });

        GeneralSliderView::SliderPtr slider_salt = sliderView->getOrCreateSlider(_("Salt"), value_range[9][0], value_range[9][1], 2);
        slider_salt->setValue(areaItem->salt());
        slider_salt->setCallback([=](double value){
            areaItem->setSalt(value);
            areaItem->notifyUpdate(); });

        GeneralSliderView::SliderPtr slider_pepper = sliderView->getOrCreateSlider(_("Pepper"), value_range[10][0], value_range[10][1], 2);
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

class ConfigDialog : public Dialog
{
public:
    ConfigDialog();

    DoubleSpinBox* optionSpins[NUM_DSPINS];
    CheckBox* flipCheck;
    ComboBox* filterCombo;

    void onResetButtonClicked();
    void storeState(Archive& archive);
    void restoreState(const Archive& archive);
};

class SubVisualEffectorItem
{
public:
    Item* item;
    BodyItem* bodyItem;
    ScopedConnection sigCheckToggledConnection;
    SgUpdate update;
    SubVisualEffectorItem(Item* item);
    void setBodyItem(BodyItem* bodyItem);
    void updateVisualization();

    virtual void enableVisualization(bool on) = 0;
    virtual void doUpdateVisualization() = 0;
};

class VEImageVisualizerItem : public Item, public SubVisualEffectorItem, public ImageableItem
{
public:
    VEImageVisualizerItem();
    virtual Item* doCloneItem(CloneMap* cloneMap) const override;
    virtual const Image* getImage() override;
    virtual SignalProxy<void()> sigImageUpdated() override;
    void setBodyItem(BodyItem* bodyItem, Camera* camera);
    virtual void enableVisualization(bool on) override;
    virtual void doUpdateVisualization() override;

    CameraPtr camera;
    ConfigDialog* configDialog;
    std::mutex mtx;
    SimulatorItem* simulatorItem;
    ImageGenerator generator;
    ScopedConnectionSet connections;
    std::shared_ptr<const Image> image;
    Signal<void()> sigImageUpdated_;

    double hue;
    double saturation;
    double value;
    double red;
    double green;
    double blue;
    bool flipped;
    double coefB;
    double coefD;
    double stdDev;
    double salt;
    double pepper;
    int filter;

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
            visualizerItem->configDialog->show();
        }
    }
}

}

namespace cnoid {

class VisualEffectorItem::Impl
{
public:
    VisualEffectorItem* self;
    BodyItem* bodyItem;
    ScopedConnection bodyItemConnection;
    ItemList<> existingSubItems;
    ItemList<> subItemsToRestore;
    SgUpdate update;
    bool isRestoringSubItems;

    Impl(VisualEffectorItem* self);
    template<class ItemType, class SensorType>
    ref_ptr<ItemType> extractMatchedSubItem(ItemList<>& items, Device* deviceInstance);
    template<class ItemType, class SensorType>
    void addVisualEffectorItem(Body* body);
    void updateSubVisualizerItems(bool forceUpdate);
};

}


void VisualEffectorItem::initializeClass(ExtensionManager* ext)
{
    ItemManager& im = ext->itemManager();

    im.registerClass<VisualEffectorItem>(N_("VisualEffectorItem"));
    im.registerClass<VEImageVisualizerItem>(N_("VEImageVisualizerItem"));

    im.addCreationPanel<VisualEffectorItem>();

    /**
       Tha following item aliases are defined for reading old project files.
       \todo Remove the aliases in newer versions.
    */
    im.addAlias<VisualEffectorItem>("VisualEffectorItem", "Body");
    im.addAlias<VEImageVisualizerItem>("VEImageVisualizer", "Body");

    ItemTreeView::instance()->customizeContextMenu<VEImageVisualizerItem>(
        [](VEImageVisualizerItem* item, MenuManager& menuManager, ItemFunctionDispatcher menuFunction) {
            menuManager.setPath("/");
            menuManager.addItem(_("Visual Effect"))->sigTriggered().connect(
                        [item](){ item->configDialog->show(); });
            menuManager.setPath("/");
            menuManager.addSeparator();
            menuFunction.dispatchAs<Item>(item);
        });

    Action* showConfig = new Action;
    showConfig->setText(_("Visual Effect"));
    contextMenu.addAction(showConfig);
    showConfig->sigTriggered().connect([&](){ onShowConfigTriggered(); });
    ext->viewManager().sigViewCreated().connect([&](View* view){ onViewCreated(view); });

    auto rootItem = RootItem::instance();
    rootItem->sigSelectedItemsChanged().connect(
        [&](const ItemList<>& selectedItems){ onSelectedItemChanged(selectedItems); });
}


VisualEffectorItem::VisualEffectorItem()
{
    impl = new Impl(this);
}


VisualEffectorItem::VisualEffectorItem(const VisualEffectorItem& org)
    : Item(org)
{
    impl = new Impl(this);
}


VisualEffectorItem::Impl::Impl(VisualEffectorItem* self)
    : self(self)
{
    bodyItem = nullptr;
    isRestoringSubItems = false;
}


VisualEffectorItem::~VisualEffectorItem()
{
    delete impl;
}


Item* VisualEffectorItem::doCloneItem(CloneMap* /* cloneMap */) const
{
    return new VisualEffectorItem(*this);
}


void VisualEffectorItem::onTreePathChanged()
{
    if(parentItem()) {
        impl->updateSubVisualizerItems(false);
    }
}


void VisualEffectorItem::onDisconnectedFromRoot()
{
    auto subItems = childItems(
        [](Item* item) -> bool { return dynamic_cast<SubVisualEffectorItem*>(item); });
    for(auto& subItem : subItems) {
        subItem->removeFromParentItem();
    }
}


void VisualEffectorItem::Impl::updateSubVisualizerItems(bool forceUpdate)
{
    auto newBodyItem = self->findOwnerItem<BodyItem>();
    bool doUpdate = forceUpdate || newBodyItem != bodyItem;
    bodyItem = newBodyItem;

    if(doUpdate) {
        existingSubItems = self->childItems(
            [](Item* item) -> bool { return dynamic_cast<SubVisualEffectorItem*>(item); });

        if(bodyItem) {
            auto body = bodyItem->body();
            int itemIndex = 0;
            addVisualEffectorItem<VEImageVisualizerItem, Camera>(body);

            bodyItemConnection = bodyItem->sigModelUpdated().connect(
                [this](int flags) {
                    if(flags & BodyItem::DeviceSetUpdate) {
                        updateSubVisualizerItems(true);
                    }
                });
        }

        for(auto& subItem : existingSubItems) {
            subItem->removeFromParentItem();
        }
        existingSubItems.clear();
    }
}


template<class ItemType, class SensorType>
ref_ptr<ItemType> VisualEffectorItem::Impl::extractMatchedSubItem(ItemList<>& items, Device* deviceInstance)
{
    ref_ptr<ItemType> matchedSubItem;
    auto it = items.begin();
    while(it != items.end()) {
        if(auto item = dynamic_pointer_cast<ItemType>(*it)) {
            if(!deviceInstance || item->name().find(deviceInstance->name()) == 0) {
                matchedSubItem = item;
                items.erase(it);
                break;
            }
        }
        ++it;
    }
    return matchedSubItem;
}


template<class ItemType, class SensorType>
void VisualEffectorItem::Impl::addVisualEffectorItem(Body* body)
{
    bool isCamera = typeid(SensorType) == typeid(Camera);
    
    for(auto& sensor : body->devices<SensorType>()) {
        if(isCamera && reinterpret_cast<Camera*>(sensor.get())->imageType() == Camera::NO_IMAGE) {
            continue;
        }
        auto item = extractMatchedSubItem<ItemType, SensorType>(existingSubItems, sensor);
        if(!item && isRestoringSubItems) {
            item = extractMatchedSubItem<ItemType, SensorType>(subItemsToRestore, nullptr);
        }
        if(!item) {
            item = new ItemType;
        }
        if(item->bodyItem != bodyItem) {
            item->setBodyItem(bodyItem, sensor);
            self->addSubItem(item);
        }
    }
}


bool VisualEffectorItem::store(Archive& archive)
{
    ListingPtr subItemListing = new Listing;

    auto subItems = childItems(
        [](Item* item) -> bool { return dynamic_cast<SubVisualEffectorItem*>(item); });
    
    for(auto& item : subItems) {
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

        subItemListing->append(subArchive);
    }

    archive.insert("sub_items", subItemListing);

    return true;
}


bool VisualEffectorItem::restore(const Archive& archive)
{
    impl->subItemsToRestore.clear();
    
    ListingPtr subItemListing = archive.findListing({ "sub_items", "subItems" });
    if(subItemListing->isValid()) {
        for(int i=0; i < subItemListing->size(); ++i) {
            auto subArchive = archive.subArchive(subItemListing->at(i)->toMapping());
            string className, itemName;
            subArchive->read("class", className);
            subArchive->read("name", itemName);
            if(ItemPtr item = ItemManager::createItem("Body", className)) {
                item->setName(itemName);
                item->restore(*subArchive);
                if(subArchive->get("is_selected", false)) {
                    item->setSelected(true);
                }
                if(subArchive->get("is_checked", false)) {
                    item->setChecked(true);
                }
                impl->subItemsToRestore.push_back(item);
            }
        }
        impl->isRestoringSubItems = true;
        archive.addPostProcess(
            [this]() {
                impl->subItemsToRestore.clear();
                impl->isRestoringSubItems = false;
            });
    }
    return true;
}


SubVisualEffectorItem::SubVisualEffectorItem(Item* item)
    : item(item),
      bodyItem(nullptr)
{
    sigCheckToggledConnection.reset(
        item->sigCheckToggled(Item::LogicalSumOfAllChecks).connect(
            [&](bool on) { enableVisualization(on); }));
}


void SubVisualEffectorItem::setBodyItem(BodyItem* bodyItem)
{
    this->bodyItem = bodyItem;
    enableVisualization(item->isChecked(Item::LogicalSumOfAllChecks));
}


void SubVisualEffectorItem::updateVisualization()
{
    if(item->isChecked(Item::LogicalSumOfAllChecks)) {
        doUpdateVisualization();
    }
}


VEImageVisualizerItem::VEImageVisualizerItem()
    : SubVisualEffectorItem(this)
{
    configDialog = new ConfigDialog;
    simulatorItem = nullptr;
    SimulationBar::instance()->sigSimulationAboutToStart().connect(
                [&](SimulatorItem* simulatorItem){ this->simulatorItem = simulatorItem; });
}


Item* VEImageVisualizerItem::doCloneItem(CloneMap* /* cloneMap */) const
{
    return nullptr;
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
        if(dynamic_cast<RangeCamera*>(camera)) {
            name += "-Image";
        }
        setName(name);
    }

    this->camera = camera;

    SubVisualEffectorItem::setBodyItem(bodyItem);
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
        hue = configDialog->optionSpins[HUE]->value();
        saturation = configDialog->optionSpins[SATURATION]->value();
        value = configDialog->optionSpins[VALUE]->value();
        red = configDialog->optionSpins[RED]->value();
        green = configDialog->optionSpins[GREEN]->value();
        blue = configDialog->optionSpins[BLUE]->value();
        flipped = configDialog->flipCheck->isChecked();
        coefB = configDialog->optionSpins[COEFB]->value();
        coefD = configDialog->optionSpins[COEFD]->value();
        stdDev = configDialog->optionSpins[STDDEV]->value();
        salt = configDialog->optionSpins[SALT]->value();
        pepper = configDialog->optionSpins[PEPPER]->value();
        filter = configDialog->filterCombo->currentIndex();

        if(simulatorItem) {
            WorldItem* worldItem = simulatorItem->findOwnerItem<WorldItem>();
            if(worldItem) {
                ItemList<VEAreaItem> areaItems = worldItem->descendantItems<VEAreaItem>();
                for(auto& areaItem : areaItems) {
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
        if(coefB < 0.0 || coefD > 1.0) {
            generator.barrelDistortion(orgImage, coefB, coefD);
        }

        image = make_shared<Image>(orgImage);
        // image = camera->sharedImage();
    } else {
        image.reset();
    }
    sigImageUpdated_();
}


bool VEImageVisualizerItem::store(Archive& archive)
{
    configDialog->storeState(archive);
    return true;
}


bool VEImageVisualizerItem::restore(const Archive& archive)
{
    configDialog->restoreState(archive);
    return true;
}


ConfigDialog::ConfigDialog()
{
    setWindowTitle(_("Effect Config"));

    static const char* label[] = {
        _("Hue"), _("Saturation"), _("Value"),
        _("Red"), _("Green"), _("Blue"),
        _("CoefB"), _("CoefD"),
        _("Std_dev"), _("Salt"), _("Pepper")
    };

    QGridLayout* gbox = new QGridLayout;
    for(int i = 0; i < NUM_DSPINS; ++i) {
        optionSpins[i] = new DoubleSpinBox;
        DoubleSpinBox* dspin = optionSpins[i];
        WidgetInfo dinfo = dspinInfo[i];
        WidgetInfo linfo = labelInfo[i];
        dspin->setRange(value_range[i][0], value_range[i][1]);
        dspin->setSingleStep(0.1);
        gbox->addWidget(new QLabel(label[i]), linfo.row, linfo.column);
        gbox->addWidget(dspin, dinfo.row, dinfo.column);
    }
    optionSpins[COEFD]->setValue(1.0);


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


void ConfigDialog::onResetButtonClicked()
{
    for(int i = 0; i < NUM_DSPINS; ++i) {
        optionSpins[i]->setValue(0.0);
    }
    optionSpins[COEFD]->setValue(1.0);
    flipCheck->setChecked(false);
    filterCombo->setCurrentIndex(0);
}


void ConfigDialog::storeState(Archive& archive)
{
    archive.write("hue", optionSpins[HUE]->value());
    archive.write("saturation", optionSpins[SATURATION]->value());
    archive.write("value", optionSpins[VALUE]->value());
    archive.write("red", optionSpins[RED]->value());
    archive.write("green", optionSpins[GREEN]->value());
    archive.write("blue", optionSpins[BLUE]->value());
    archive.write("coef_b", optionSpins[COEFB]->value());
    archive.write("coef_d", optionSpins[COEFD]->value());
    archive.write("std_dev", optionSpins[STDDEV]->value());
    archive.write("salt", optionSpins[SALT]->value());
    archive.write("pepper", optionSpins[PEPPER]->value());
    archive.write("flip", flipCheck->isChecked());
    archive.write("filter", filterCombo->currentIndex());
}


void ConfigDialog::restoreState(const Archive& archive)
{
    optionSpins[HUE]->setValue(archive.get("hue", 0.0));
    optionSpins[SATURATION]->setValue(archive.get("saturation", 0.0));
    optionSpins[VALUE]->setValue(archive.get("value", 0.0));
    optionSpins[RED]->setValue(archive.get("red", 0.0));
    optionSpins[GREEN]->setValue(archive.get("green", 0.0));
    optionSpins[BLUE]->setValue(archive.get("blue", 0.0));
    optionSpins[COEFB]->setValue(archive.get("coef_b", 0.0));
    optionSpins[COEFD]->setValue(archive.get("coef_d", 1.0));
    optionSpins[STDDEV]->setValue(archive.get("std_dev", 0.0));
    optionSpins[SALT]->setValue(archive.get("salt", 0.0));
    optionSpins[PEPPER]->setValue(archive.get("pepper", 0.0));
    flipCheck->setChecked(archive.get("flip", false));
    filterCombo->setCurrentIndex(archive.get("filter", 0));
}
