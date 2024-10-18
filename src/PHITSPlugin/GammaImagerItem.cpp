/**
   @author Kenta Suzuki
*/

#include "GammaImagerItem.h"
#include <cnoid/Archive>
#include <cnoid/BasicSensors>
#include <cnoid/BodyItem>
#include <cnoid/Camera>
#include <cnoid/ConnectionSet>
#include <cnoid/ExecutablePath>
#include <cnoid/PutPropertyFunction>
#include <cnoid/ItemManager>
#include <cnoid/ItemTreeView>
#include <cnoid/ImageableItem>
#include <cnoid/MenuManager>
#include <cnoid/PutPropertyFunction>
#include <cnoid/RootItem>
#include <cnoid/stdx/filesystem>
#include <cnoid/UTF8>
#include <QDateTime>
#include <QFile>
#include <QTextStream>
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
    GammaImageGenerator generator;
    EnergyFilter* filter;
    ScopedConnectionSet connections;
    std::shared_ptr<const Image> image;
    Signal<void()> sigImageUpdated_;

    PHITSRunner phitsRunner;
    PHITSWriter phitsWriter;
    ComptonCamera* comptonCamera;
    PinholeCamera* pinholeCamera;
    int maxcas;
    int maxbch;
    bool is_message_checked;

    void setCamera(Camera* camera);
    void start(bool checked);

    virtual bool store(Archive& archive) override;
    virtual bool restore(const Archive& archive) override;

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
    string default_nuclide_table_file;
    string default_element_table_file;
    string default_energy_filter_file;

    void onPositionChanged();
};

}


void GammaImagerItem::initializeClass(ExtensionManager* ext)
{
    ext->itemManager()
        .registerClass<GammaImagerItem>(N_("GammaImagerItem"))
        .addCreationPanel<GammaImagerItem>();
    ext->itemManager()
        .registerClass<GammaImageVisualizerItem>(N_("GammaImageVisualizerItem"));

    ItemTreeView::customizeContextMenu<GammaImageVisualizerItem>(
        [](GammaImageVisualizerItem* item, MenuManager& menuManager, ItemFunctionDispatcher menuFunction) {
            menuManager.setPath("/").setPath(_("PHITS"));
            menuManager.addItem(_("Start"))->sigTriggered().connect(
                        [item](){ item->start(true); });
            menuManager.addItem(_("Stop"))->sigTriggered().connect(
                        [item](){ item->start(false); });
        //    menuManager.addItem(_("Energy Filter"))->sigTriggered().connect(
        //        [item](){ item->filter->showConfigDialog(); });
           menuManager.setPath("/");
           menuManager.addSeparator();
           menuFunction.dispatchAs<Item>(item);
        });
}


void GammaImagerItem::setDefaultEnergyFilterFile(const string& filename)
{
    if(filename != impl->default_energy_filter_file) {
        impl->default_energy_filter_file = filename;
        for(size_t i = 0; i < impl->subItems.size(); ++i) {
            GammaImageVisualizerItem* item = dynamic_cast<GammaImageVisualizerItem*>(impl->subItems[i]);
            if(item) {
                item->filter->load(filename);
            }
        }
    }
}


string GammaImagerItem::defaultNuclideTableFile() const
{
    return impl->default_nuclide_table_file;
}


string GammaImagerItem::defaultElementTableFile() const
{
    return impl->default_element_table_file;
}


string GammaImagerItem::defaultEnergyFilterFile() const
{
    return impl->default_energy_filter_file;
}


GammaImagerItem::GammaImagerItem()
{
    impl = new Impl(this);
}


GammaImagerItem::Impl::Impl(GammaImagerItem* self)
    : self(self)
{
    default_nuclide_table_file = toUTF8((shareDirPath() / "default" / "nuclides.yaml").string());
    default_element_table_file = toUTF8((shareDirPath() / "default" / "elements.yaml").string());
    default_energy_filter_file = toUTF8((shareDirPath() / "default" / "filters.yaml").string());
}


GammaImagerItem::GammaImagerItem(const GammaImagerItem& org)
    : Item(org)
{
    impl = new Impl(this);
}


GammaImagerItem::Impl::Impl(GammaImagerItem* self, const Impl& org)
    : self(self),
      default_nuclide_table_file(org.default_nuclide_table_file),
      default_element_table_file(org.default_element_table_file),
      default_energy_filter_file(org.default_energy_filter_file)
{

}


GammaImagerItem::~GammaImagerItem()
{
    delete impl;
}


Item* GammaImagerItem::doCloneItem(CloneMap* cloneMap) const
{
    return new GammaImagerItem(*this);
}


void GammaImagerItem::doPutProperties(PutPropertyFunction& putProperty)
{
    FilePathProperty nuclideFileProperty(
                impl->default_nuclide_table_file, { _("Nuclide definition file (*.yaml)") });
    putProperty(_("Default nuclide table"), nuclideFileProperty,
                [&](const string& filename){ impl->default_nuclide_table_file = filename; return true; });
    FilePathProperty elementFileProperty(
                impl->default_element_table_file, { _("Element definition file (*.yaml)") });
    putProperty(_("Default element table"), elementFileProperty,
                [&](const string& filename){ impl->default_element_table_file = filename; return true; });
    FilePathProperty filterFileProperty(
                impl->default_energy_filter_file, { _("Energy filter definition file (*.yaml)") });
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
                            gammaImageVisualizerItem->filter->load(default_energy_filter_file);
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
    archive.writeRelocatablePath("default_nuclide_table_file", impl->default_nuclide_table_file);
    archive.writeRelocatablePath("default_element_table_file", impl->default_element_table_file);
    archive.writeRelocatablePath("default_energy_filter_file", impl->default_energy_filter_file);

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
            vitem->store(*subArchive);
        }

        subItems->append(subArchive);
    }

    archive.insert("sub_items", subItems);

    return true;
}


bool GammaImagerItem::restore(const Archive& archive)
{
    archive.readRelocatablePath("default_nuclide_table_file", impl->default_nuclide_table_file);
    archive.readRelocatablePath("default_element_table_file", impl->default_element_table_file);
    archive.readRelocatablePath("default_energy_filter_file", impl->default_energy_filter_file);

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
                    vitem->restore(*subArchive);
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
            [&](bool checked){ enableVisualization(checked); }));
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
      filter(new EnergyFilter)
{
    maxcas = 1000;
    maxbch = 2;
    is_message_checked = true;
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
    setCamera(camera);

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


void GammaImageVisualizerItem::setCamera(Camera* camera)
{
    comptonCamera = dynamic_cast<ComptonCamera*>(camera);
    pinholeCamera = dynamic_cast<PinholeCamera*>(camera);
    phitsRunner.setCamera(camera);
    phitsWriter.setCamera(camera);
}


void GammaImageVisualizerItem::start(bool checked)
{
    if(checked) {
        filesystem::path homeDirPath(fromUTF8(getenv("HOME")));
        QDateTime recordingStartTime = QDateTime::currentDateTime();
        string suffix = recordingStartTime.toString("-yyyy-MM-dd-hh-mm-ss").toStdString();
        string phits_dir = toUTF8((homeDirPath / "phits_ws" / ("phits" + suffix).c_str()).string());
        filesystem::path phitsDirPath(fromUTF8(phits_dir));
        if(!filesystem::exists(phitsDirPath)) {
            filesystem::create_directories(phitsDirPath);
        }

        if(comptonCamera || pinholeCamera) {
            string filename = toUTF8((phitsDirPath / "phits.inp").string());
            filesystem::path filePath(fromUTF8(filename));
            filesystem::path parentDirPath(filePath.parent_path());

            string filename0;
            GammaData::CalcInfo calcInfo;
            calcInfo.maxcas = maxcas;
            calcInfo.maxbch = maxbch;
            if(comptonCamera) {
                calcInfo.inputMode = GammaData::COMPTON;
                filename0 = toUTF8((parentDirPath / "flux_cross_dmp.out").string());
            } else if(pinholeCamera) {
                calcInfo.inputMode = GammaData::PINHOLE;
                filename0 = toUTF8((parentDirPath / "cross_xz.out").string());
            }

            GammaImagerItem* parentItem = dynamic_cast<GammaImagerItem*>(this->parentItem());

            phitsWriter.setDefaultNuclideTableFile(parentItem->defaultNuclideTableFile());
            phitsWriter.setDefaultElementTableFile(parentItem->defaultElementTableFile());
            writeTextFile(filename, phitsWriter.writePHITS(calcInfo));
            phitsRunner.setEnergy(phitsWriter.energy());
            phitsRunner.setReadStandardOutput(filename0, calcInfo.inputMode);
            phitsRunner.startPHITS(filename.c_str());
        }
    } else {
        if(comptonCamera || pinholeCamera) {
            phitsRunner.stop();
        }
    }
}


void GammaImageVisualizerItem::doPutProperties(PutPropertyFunction& putProperty)
{
    putProperty.min(1).max(INT_MAX)(_("maxcas"), maxcas, changeProperty(maxcas));
    putProperty.min(1).max(1000000)(_("maxbch"), maxbch, changeProperty(maxbch));
    putProperty(_("Put messages"), is_message_checked,
                [&](bool value){ is_message_checked = value;
                phitsRunner.putMessages(is_message_checked);
                return true; });
}


bool GammaImageVisualizerItem::store(Archive& archive)
{
    archive.write("maxcas", maxcas);
    archive.write("maxbch", maxbch);
    archive.write("put_messages", is_message_checked);
    return true;
}


bool GammaImageVisualizerItem::restore(const Archive& archive)
{
    maxcas = archive.get("maxcas", 0);
    maxbch = archive.get("maxbch", 0);
    is_message_checked = archive.get("put_messages", true);
    return true;
}
