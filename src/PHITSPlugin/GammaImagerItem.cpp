/**
   @author Kenta Suzuki
*/

#include "GammaImagerItem.h"
#include <cnoid/Archive>
#include <cnoid/BasicSensors>
#include <cnoid/BodyItem>
#include <cnoid/Button>
#include <cnoid/Camera>
#include <cnoid/CheckBox>
#include <cnoid/ConnectionSet>
#include <cnoid/Dialog>
#include <cnoid/ExecutablePath>
#include <cnoid/FileDialog>
#include <cnoid/PutPropertyFunction>
#include <cnoid/ItemManager>
#include <cnoid/ItemTreeView>
#include <cnoid/ImageableItem>
#include <cnoid/MenuManager>
#include <cnoid/PutPropertyFunction>
#include <cnoid/Separator>
#include <cnoid/SpinBox>
#include <cnoid/stdx/filesystem>
#include <cnoid/UTF8>
#include <QDateTime>
#include <QDialogButtonBox>
#include <QLabel>
#include <QTextStream>
#include <QVBoxLayout>
#include "ComptonCamera.h"
#include "EnergyFilter.h"
#include "GammaCamera.h"
#include "GammaImageGenerator.h"
#include "PHITSRunner.h"
#include "PHITSWriter.h"
#include "PinholeCamera.h"
#include "gettext.h"

#ifndef _MSC_VER
#include <libgen.h>
#endif

using namespace std;
using namespace cnoid;
namespace filesystem = cnoid::stdx::filesystem;

namespace {

bool writeTextFile(const string& filename, const string& text)
{
    if(!text.empty()) {
        QFile file(filename.c_str());
        if(!file.open(QIODevice::WriteOnly)) {
            return false;
        }
        QTextStream qts(&file);
        qts << text.c_str();
        file.close();
    }
    return true;
}

class ConfigDialog : public Dialog
{
public:
    ConfigDialog();

    SpinBox* maxCasSpin;
    SpinBox* maxBchSpin;
    PHITSRunner phitsRunner;
    PHITSWriter phitsWriter;
    ComptonCamera* ccamera;
    PinholeCamera* pcamera;
    CheckBox* messageCheck;
    string defaultNuclideTableFile_;
    string defaultElementTableFile_;

    void setCamera(Camera* camera);
    void onItemTriggered(const bool& on);
    void storeState(Archive& archive);
    void restoreState(const Archive& archive);
};

class GammaImagerItemBase
{
public:
    Item* imagerItem;
    BodyItem* bodyItem;
    ScopedConnection sigCheckToggledConnection;
    GammaImagerItemBase(Item* imagerItem);
    void setBodyItem(BodyItem* bodyItem);
    void updateVisualization();

    virtual void enableVisualization(bool on) = 0;
    virtual void doUpdateVisualization() = 0;
};

class GammaImageVisualizerItem : public Item, public ImageableItem, public GammaImagerItemBase
{
public:
    GammaImageVisualizerItem();
    virtual const Image* getImage() override;
    virtual SignalProxy<void()> sigImageUpdated() override;
    void setBodyItem(BodyItem* bodyItem, Camera* camera);
    virtual void enableVisualization(bool on) override;
    virtual void doUpdateVisualization() override;

    CameraPtr camera;
    ConfigDialog* config;
    GammaImageGenerator generator;
    EnergyFilter* filter;
    ScopedConnectionSet connections;
    std::shared_ptr<const Image> image;
    Signal<void()> sigImageUpdated_;

    void setDefaultNuclideTableFile(const string& filename);
    void setDefaultElementTableFile(const string& filename);

protected:
    virtual void doPutProperties(PutPropertyFunction& putProperty) override;
};

}


namespace cnoid {

class GammaImagerItem::Impl
{
public:
    GammaImagerItem* self;

    Impl(GammaImagerItem* self);
    Impl(GammaImagerItem* self, const Impl& org);

    BodyItem* bodyItem;
    vector<Item*> subItems;
    vector<ItemPtr> restoredSubItems;
    string defaultEnergyFilterFile;

    void onPositionChanged();
};

}


void GammaImagerItem::initializeClass(ExtensionManager* ext)
{
    ItemManager& im = ext->itemManager();
    im.registerClass<GammaImagerItem>(N_("GammaImagerItem"));
    im.addCreationPanel<GammaImagerItem>();

    im.registerClass<GammaImageVisualizerItem>(N_("GammaImageVisualizerItem"));

    ItemTreeView::instance()->customizeContextMenu<GammaImageVisualizerItem>(
        [](GammaImageVisualizerItem* item, MenuManager& menuManager, ItemFunctionDispatcher menuFunction) {
            menuManager.setPath("/").setPath(_("PHITS"));
            menuManager.addItem(_("Start"))->sigTriggered().connect(
                        [item](){ item->config->onItemTriggered(true); });
            menuManager.addItem(_("Stop"))->sigTriggered().connect(
                        [item](){ item->config->onItemTriggered(false); });
            menuManager.setPath("/");
            menuManager.addItem(_("Configuration of PHITS"))->sigTriggered().connect(
                [item](){ item->config->show(); });
//            menuManager.addItem(_("Energy Filter"))->sigTriggered().connect(
//                [item](){ item->filter->showConfigDialog(); });
//            menuManager.setPath("/");
//            menuManager.addSeparator();
//            menuFunction.dispatchAs<Item>(item);
        });
}


void GammaImagerItem::setDefaultEnergyFilterFile(const string& filename)
{
    if(filename != impl->defaultEnergyFilterFile) {
        impl->defaultEnergyFilterFile = filename;
        for(size_t i = 0; i < impl->subItems.size(); ++i) {
            GammaImageVisualizerItem* item = dynamic_cast<GammaImageVisualizerItem*>(impl->subItems[i]);
            if(item) {
                item->filter->load(filename);
            }
        }
    }
}


string GammaImagerItem::defaultEnergyFilterFile() const
{
    return impl->defaultEnergyFilterFile;
}


GammaImagerItem::GammaImagerItem()
{
    impl = new Impl(this);
}


GammaImagerItem::Impl::Impl(GammaImagerItem* self)
    : self(self)
{
    defaultEnergyFilterFile = toUTF8((shareDirPath() / "default" / "filters.yaml").string());
}


GammaImagerItem::GammaImagerItem(const GammaImagerItem& org)
    : Item(org)
{
    impl = new Impl(this);
}


GammaImagerItem::Impl::Impl(GammaImagerItem* self, const Impl& org)
    : self(self),
      defaultEnergyFilterFile(org.defaultEnergyFilterFile)
{

}


GammaImagerItem::~GammaImagerItem()
{
    delete impl;
}


Item* GammaImagerItem::doDuplicate() const
{
    return new GammaImagerItem(*this);
}


void GammaImagerItem::doPutProperties(PutPropertyFunction& putProperty)
{
    FilePathProperty filterFileProperty(
                impl->defaultEnergyFilterFile, { _("Energy filter definition file (*.yaml)") });
    putProperty(_("Default energy filter"), filterFileProperty,
                [&](const string& filename){ setDefaultEnergyFilterFile(filename); return true; });
}


void GammaImagerItem::onPositionChanged()
{
    if(parentItem()) {
        impl->onPositionChanged();
    }
}


void GammaImagerItem::Impl::onPositionChanged()
{
    BodyItem* newBodyItem = self->findOwnerItem<BodyItem>();
    if(newBodyItem != bodyItem) {
        bodyItem = newBodyItem;
        for(size_t i = 0; i < subItems.size(); i++) {
            subItems[i]->removeFromParentItem();
        }
        subItems.clear();

        int n = restoredSubItems.size();
        int j = 0;

        if(bodyItem) {
            Body* body = bodyItem->body();

            DeviceList<Camera> camera = body->devices<Camera>();
            for(size_t i = 0; i < camera.size(); ++i) {
                Camera* tmpCamera = camera[i];
                GammaCamera* gcamera = dynamic_cast<GammaCamera*>(tmpCamera);
                if(gcamera) {
                    if(camera[i]->imageType()!= Camera::NO_IMAGE) {
                        GammaImageVisualizerItem* gammaImageVisualizerItem =
                                j<n ? dynamic_cast<GammaImageVisualizerItem*>(restoredSubItems[j++].get()) : new GammaImageVisualizerItem;
                        if(gammaImageVisualizerItem) {
                            gammaImageVisualizerItem->setBodyItem(bodyItem, camera[i]);
                            gammaImageVisualizerItem->filter->load(defaultEnergyFilterFile);
                            self->addSubItem(gammaImageVisualizerItem);
                            subItems.push_back(gammaImageVisualizerItem);
                        }
                    }
                }
            }
        }

        restoredSubItems.clear();
    }
}


void GammaImagerItem::onDisconnectedFromRoot()
{
    for(size_t i = 0; i < impl->subItems.size(); i++) {
        impl->subItems[i]->removeFromParentItem();
    }
    impl->subItems.clear();
}


bool GammaImagerItem::store(Archive& archive)
{
    ListingPtr subItems = new Listing;

    for(size_t i = 0; i < impl->subItems.size(); i++) {
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
        GammaImageVisualizerItem* vitem = dynamic_cast<GammaImageVisualizerItem*>(item);
        if(vitem) {
            vitem->filter->storeState(*subArchive);
            ConfigDialog* config = vitem->config;
            config->storeState(*subArchive);
            subArchive->write("default_nuclide_table_file", archive.getRelocatablePath(config->defaultNuclideTableFile_));
            subArchive->write("default_element_table_file", archive.getRelocatablePath(config->defaultElementTableFile_));
        }

        subItems->append(subArchive);
    }

    archive.insert("sub_items", subItems);

    return true;
}


bool GammaImagerItem::restore(const Archive& archive)
{
    impl->restoredSubItems.clear();

    ListingPtr subItems = archive.findListing("sub_items");
    if(!subItems->isValid()) {
        subItems = archive.findListing("subItems"); // Old
    }
    if(subItems->isValid()) {
        for(int i = 0; i < subItems->size(); i++) {
            Archive* subArchive = dynamic_cast<Archive*>(subItems->at(i)->toMapping());
            string className, itemName;
            subArchive->read("class", className);
            subArchive->read("name", itemName);
            if(ItemPtr item = ItemManager::createItem("PHITS", className)) {
                item->setName(itemName);
                item->restore(*subArchive);
                if(subArchive->get("is_selected", false)) {
                    item->setSelected(true);
                }
                if(subArchive->get("is_checked", false)) {
                    item->setChecked(true);
                }
                Item* tmpItem = item;
                GammaImageVisualizerItem* vitem = dynamic_cast<GammaImageVisualizerItem*>(tmpItem);
                if(vitem) {
                    vitem->filter->restoreState(*subArchive);
                    ConfigDialog* config = vitem->config;
                    config->restoreState(*subArchive);
                    string default_nuclide_table_file;
                    string default_element_table_file;
                    subArchive->read("default_nuclide_table_file", default_nuclide_table_file);
                    subArchive->read("default_element_table_file", default_element_table_file);
                    config->defaultNuclideTableFile_ = archive.resolveRelocatablePath(default_nuclide_table_file);
                    config->defaultElementTableFile_ = archive.resolveRelocatablePath(default_element_table_file);
                }

                impl->restoredSubItems.push_back(item);
            }
        }
    }
    return true;
}


GammaImagerItemBase::GammaImagerItemBase(Item* imagerItem)
    : imagerItem(imagerItem),
      bodyItem(nullptr)
{
    sigCheckToggledConnection.reset(
        imagerItem->sigCheckToggled(Item::LogicalSumOfAllChecks).connect(
            [&](bool on){ enableVisualization(on); }));
}


void GammaImagerItemBase::setBodyItem(BodyItem* bodyItem)
{
    this->bodyItem = bodyItem;
    enableVisualization(imagerItem->isChecked(Item::LogicalSumOfAllChecks));
}


void GammaImagerItemBase::updateVisualization()
{
    if(imagerItem->isChecked(Item::LogicalSumOfAllChecks)) {
        doUpdateVisualization();
    }
}


GammaImageVisualizerItem::GammaImageVisualizerItem()
    : GammaImagerItemBase(this),
      config(new ConfigDialog),
      filter(new EnergyFilter)
{

}


const Image* GammaImageVisualizerItem::getImage()
{
    return image.get();
}


SignalProxy<void()> GammaImageVisualizerItem::sigImageUpdated()
{
    return sigImageUpdated_;
}


void GammaImageVisualizerItem::setBodyItem(BodyItem* bodyItem, Camera* camera)
{
    if(name().empty()) {
        string name = camera->name();
        setName(name);
    }

    this->camera = camera;
    config->setCamera(camera);

    GammaImagerItemBase::setBodyItem(bodyItem);
}


void GammaImageVisualizerItem::enableVisualization(bool on)
{
    connections.disconnect();

    if(camera && on) {
        connections.add(
            camera->sigStateChanged().connect(
                [&](){ doUpdateVisualization(); }));

        doUpdateVisualization();
    }
}


void GammaImageVisualizerItem::doUpdateVisualization()
{
    if(camera) {
        const Image& orgImage = *camera->sharedImage();
        if(!orgImage.empty()) {
            shared_ptr<Image> tmpImage = make_shared<Image>(orgImage);
            generator.generateImage(camera, tmpImage);
            image = tmpImage;
        } else {
            image = camera->sharedImage();
        }
    } else {
        image.reset();
    }
    sigImageUpdated_();
}


void GammaImageVisualizerItem::setDefaultNuclideTableFile(const string& filename)
{
    config->defaultNuclideTableFile_ = filename;
}


void GammaImageVisualizerItem::setDefaultElementTableFile(const string& filename)
{
    config->defaultElementTableFile_ = filename;
}


void GammaImageVisualizerItem::doPutProperties(PutPropertyFunction& putProperty)
{
    FilePathProperty nuclideFileProperty(
                config->defaultNuclideTableFile_, { _("Nuclide definition file (*.yaml)") });
    putProperty(_("Default nuclide table"), nuclideFileProperty,
                [&](const string& filename){ setDefaultNuclideTableFile(filename); return true; });
    FilePathProperty elementFileProperty(
                config->defaultElementTableFile_, { _("Element definition file (*.yaml)") });
    putProperty(_("Default element table"), elementFileProperty,
                [&](const string& filename){ setDefaultElementTableFile(filename); return true; });
}


ConfigDialog::ConfigDialog()
{
    setWindowTitle(_("Configuration of PHITS"));

    maxCasSpin = new SpinBox;
    maxCasSpin->setRange(1, INT_MAX);
    maxCasSpin->setValue(1000);
    maxBchSpin = new SpinBox;
    maxBchSpin->setRange(1, 1000000);
    maxBchSpin->setValue(2);

    messageCheck = new CheckBox;
    messageCheck->setChecked(true);
    messageCheck->setText(_("Put messages"));
    messageCheck->sigToggled().connect([&](bool on){ phitsRunner.putMessages(on); });

    QGridLayout* gbox = new QGridLayout;
    int index = 0;
    gbox->addWidget(new QLabel(_("maxcas")), index, 0);
    gbox->addWidget(maxCasSpin, index, 1);
    gbox->addWidget(new QLabel(_("maxbch")), index, 2);
    gbox->addWidget(maxBchSpin, index++, 3);
    gbox->addWidget(messageCheck, index++, 2, 1, 2);

    auto buttonBox = new QDialogButtonBox(this);
    auto okButton = new PushButton(_("&Ok"));
    buttonBox->addButton(okButton, QDialogButtonBox::AcceptRole);
    connect(buttonBox, &QDialogButtonBox::accepted, [this](){ this->accept(); });

    QVBoxLayout* vbox = new QVBoxLayout;
    vbox->addLayout(new HSeparatorBox(new QLabel("PHITS")));
    vbox->addLayout(gbox);
    vbox->addWidget(new HSeparator);
    vbox->addWidget(buttonBox);
    setLayout(vbox);

    defaultNuclideTableFile_ = toUTF8((shareDirPath() / "default" / "nuclides.yaml").string());
    defaultElementTableFile_ = toUTF8((shareDirPath() / "default" / "elements.yaml").string());
}


void ConfigDialog::onItemTriggered(const bool& on)
{
    if(on) {
        filesystem::path homeDir(getenv("HOME"));
        QDateTime recordingStartTime = QDateTime::currentDateTime();
        string suffix = recordingStartTime.toString("-yyyy-MM-dd-hh-mm-ss").toStdString();
        string phitsDirPath = toUTF8((homeDir / "phits_ws" / ("phits" + suffix).c_str()).string());
        filesystem::path dir(fromUTF8(phitsDirPath));
        if(!filesystem::exists(dir)) {
            filesystem::create_directories(dir);
        }

        if(ccamera || pcamera) {
            string filename = toUTF8((dir / "phits").string()) + ".inp";
            filesystem::path filePath(filename);
            filesystem::path dir(filePath.parent_path());

            string filename0;
            GammaData::CalcInfo calcInfo;
            calcInfo.maxcas = maxCasSpin->value();
            calcInfo.maxbch = maxBchSpin->value();
            if(ccamera) {
                calcInfo.inputMode = GammaData::COMPTON;
                filename0 = toUTF8((dir / "flux_cross_dmp.out").string());
            } else if(pcamera) {
                calcInfo.inputMode = GammaData::PINHOLE;
                filename0 = toUTF8((dir / "cross_xz.out").string());
            }

            phitsWriter.setDefaultNuclideTableFile(defaultNuclideTableFile_);
            phitsWriter.setDefaultElementTableFile(defaultElementTableFile_);
            writeTextFile(filename, phitsWriter.writePHITS(calcInfo));
            phitsRunner.setEnergy(phitsWriter.energy());
            phitsRunner.setReadStandardOutput(filename0, calcInfo.inputMode);
            phitsRunner.startPHITS(filename.c_str());
        }
    } else {
        if(ccamera || pcamera) {
            phitsRunner.stop();
        }
    }
}


void ConfigDialog::setCamera(Camera* camera)
{
    ccamera = dynamic_cast<ComptonCamera*>(camera);
    pcamera = dynamic_cast<PinholeCamera*>(camera);
    phitsRunner.setCamera(camera);
    phitsWriter.setCamera(camera);
}


void ConfigDialog::storeState(Archive& archive)
{
    archive.write("maxcas", maxCasSpin->value());
    archive.write("maxbch", maxBchSpin->value());
    archive.write("put_messages", messageCheck->isChecked());
}


void ConfigDialog::restoreState(const Archive& archive)
{
    maxCasSpin->setValue(archive.get("maxcas", 0));
    maxBchSpin->setValue(archive.get("maxbch", 0));
    messageCheck->setChecked(archive.get("put_messages", true));
}
