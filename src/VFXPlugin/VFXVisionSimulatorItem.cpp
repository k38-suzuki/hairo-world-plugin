/**
   @author Kenta Suzuki
*/

#include "VFXVisionSimulatorItem.h"
#include <cnoid/Body>
#include <cnoid/ConnectionSet>
#include <cnoid/ItemManager>
#include <cnoid/WorldItem>
#include <cnoid/Archive>
#include <cnoid/PutPropertyFunction>
#include <cnoid/DeviceList>
#include <cnoid/SimulatorItem>
#include <cnoid/MultiColliderItem>
#include <mutex>
#include "VisualFilter.h"
#include "NoisyCamera.h"
#include "VFXEventReader.h"
#include "gettext.h"

using namespace std;
using namespace cnoid;

namespace cnoid {

class VFXVisionSimulatorItem::Impl
{
public:
    VFXVisionSimulatorItem* self;

    Impl(VFXVisionSimulatorItem* self);
    Impl(VFXVisionSimulatorItem* self, const Impl& org);

    bool initializeSimulation(SimulatorItem* simulatorItem);
    void onCameraStateChanged(Camera* camera);

    DeviceList<Camera> cameras;
    ItemList<MultiColliderItem> colliders;
    SimulatorItem* simulatorItem;
    ConnectionSet connections;
    std::mutex convertMutex;
    VisualFilter filter;
    string vfx_event_file_path;
    vector<VFXEvent> events;
};

}


void VFXVisionSimulatorItem::initializeClass(ExtensionManager* ext)
{
    ext->itemManager()
        .registerClass<VFXVisionSimulatorItem, GLVisionSimulatorItem>(N_("VFXVisionSimulatorItem"))
        .addCreationPanel<VFXVisionSimulatorItem>();
}


VFXVisionSimulatorItem::VFXVisionSimulatorItem()
    : GLVisionSimulatorItem()
{
    impl = new Impl(this);
}


VFXVisionSimulatorItem::Impl::Impl(VFXVisionSimulatorItem* self)
    : self(self),
      vfx_event_file_path("")
{
    self->setName("VFXVisionSimulator");

    cameras.clear();
    colliders.clear();
    simulatorItem = nullptr;
    events.clear();
}


VFXVisionSimulatorItem::VFXVisionSimulatorItem(const VFXVisionSimulatorItem& org)
    : GLVisionSimulatorItem(org),
      impl(new Impl(this, *org.impl))
{

}


VFXVisionSimulatorItem::Impl::Impl(VFXVisionSimulatorItem* self, const Impl& org)
    : self(self)
{
    cameras.clear();
    colliders.clear();
    vfx_event_file_path = org.vfx_event_file_path;
}


VFXVisionSimulatorItem::~VFXVisionSimulatorItem()
{
    delete impl;
}


bool VFXVisionSimulatorItem::initializeSimulation(SimulatorItem* simulatorItem)
{
    if(!GLVisionSimulatorItem::initializeSimulation(simulatorItem)) {
        return false;
    }
    return impl->initializeSimulation(simulatorItem);
}


bool VFXVisionSimulatorItem::Impl::initializeSimulation(SimulatorItem* simulatorItem)
{
    cameras.clear();
    colliders.clear();
    this->simulatorItem = simulatorItem;
    events.clear();

    if(!vfx_event_file_path.empty()) {
        VFXEventReader reader;
        if(reader.load(vfx_event_file_path)) {
            events = reader.events();
        }
    }

    const vector<SimulationBody*>& simBodies = simulatorItem->simulationBodies();
    for(auto& simBody : simBodies) {
        Body* body = simBody->body();
        cameras << body->devices();
    }

    WorldItem* worldItem = simulatorItem->findOwnerItem<WorldItem>();
    if(worldItem) {
        ItemList<MultiColliderItem> list = worldItem->descendantItems<MultiColliderItem>();
        for(auto& collider : list) {
            if(collider->colliderType() == MultiColliderItem::VFX) {
                colliders.push_back(collider);
            }
        }
    }

    for(auto& camera : cameras) {
        connections.add(camera->sigStateChanged().connect([&, camera](){ onCameraStateChanged(camera); }));
    }

    return true;
}


void VFXVisionSimulatorItem::finalizeSimulation()
{
    GLVisionSimulatorItem::finalizeSimulation();
    impl->connections.disconnect();
}


void VFXVisionSimulatorItem::Impl::onCameraStateChanged(Camera* camera)
{
    double current_time = simulatorItem->currentTime();

    Link* link = camera->link();
    double hue = 0.0;
    double saturation = 0.0;
    double value = 0.0;
    double red = 0.0;
    double green = 0.0;
    double blue = 0.0;
    double coef_b = 0.0;
    double coef_d = 1.0;
    double std_dev = 0.0;
    double salt_amount = 0.0;
    double salt_chance = 0.0;
    double pepper_amount = 0.0;
    double pepper_chance = 0.0;
    double mosaic_chance = 0.0;
    int kernel = 16;

    NoisyCamera* noisyCamera = dynamic_cast<NoisyCamera*>(camera);
    if(noisyCamera) {
        hue = noisyCamera->hsv()[0];
        saturation = noisyCamera->hsv()[1];
        value = noisyCamera->hsv()[2];
        red = noisyCamera->rgb()[0];
        green = noisyCamera->rgb()[1];
        blue = noisyCamera->rgb()[2];
        coef_b = noisyCamera->coefB();
        coef_d = noisyCamera->coefD();
        std_dev = noisyCamera->stdDev();
        salt_amount = noisyCamera->saltAmount();
        salt_chance = noisyCamera->saltChance();
        pepper_amount = noisyCamera->pepperAmount();
        pepper_chance = noisyCamera->pepperChance();
        mosaic_chance = noisyCamera->mosaicChance();
        kernel = noisyCamera->kernel();
    }

    for(auto& collider : colliders) {
        bool is_link_collided = false;
        if(collision(collider, link->T().translation())) {
            hue = collider->hsv()[0];
            saturation = collider->hsv()[1];
            value = collider->hsv()[2];
            red = collider->rgb()[0];
            green = collider->rgb()[1];
            blue = collider->rgb()[2];
            coef_b = collider->coefB();
            coef_d = collider->coefD();
            std_dev = collider->stdDev();
            salt_amount = collider->saltAmount();
            salt_chance = collider->saltChance();
            pepper_amount = collider->pepperAmount();
            pepper_chance = collider->pepperChance();
            mosaic_chance = collider->mosaicChance();
            kernel = collider->kernel();

            is_link_collided = true;
        }

        for(auto& event : events) {
            for(auto& target_collider : event.targetColliders()) {
                if(target_collider == collider->name() && is_link_collided) {
                    double begin_time = event.beginTime();
                    double end_time = std::max({ event.endTime(), event.beginTime() + event.duration() });
                    bool is_event_enabled = current_time >= begin_time ? true: false;
                    is_event_enabled = current_time >= end_time ? false : is_event_enabled;
                    Vector2 cycle = event.cycle();
                    if(cycle[0] > 0.0 && cycle[1] > 0.0) {
                        if(!is_event_enabled && event.isEnabled()) {
                            event.setBeginTime(end_time + cycle[1]);
                            event.setEndTime(end_time + cycle[0] + cycle[1]);
                        }
                    }
                    event.setEnabled(is_event_enabled);

                    if(is_event_enabled) {
                        hue = event.hsv()[0] > 0.0 ? event.hsv()[0] : hue;
                        saturation = event.hsv()[1] > 0.0 ? event.hsv()[1] : saturation;
                        value = event.hsv()[2] > 0.0 ? event.hsv()[2] : value;
                        red = event.rgb()[0] > 0.0 ? event.rgb()[0] : red;
                        green = event.rgb()[1] > 0.0 ? event.rgb()[1] : green;
                        blue = event.rgb()[2] > 0.0 ? event.rgb()[2] : blue;
                        coef_b = event.coefB() < 0.0 ? event.coefB() : coef_b;
                        coef_d = event.coefD() > 1.0 ? event.coefD() : coef_d;
                        std_dev = event.stdDev() > 0.0 ? event.stdDev() : std_dev;
                        salt_amount = event.saltAmount() > 0.0 ? event.saltAmount() : salt_amount;
                        salt_chance = event.saltChance() > 0.0 ? event.saltChance() : salt_chance;
                        pepper_amount = event.pepperAmount() > 0.0 ? event.pepperAmount() : pepper_amount;
                        pepper_chance = event.pepperChance() > 0.0 ? event.pepperChance() : pepper_chance;
                        mosaic_chance = event.mosaicChance() > 0.0 ? event.mosaicChance() : mosaic_chance;
                        kernel = event.kernel() != 16 ? event.kernel() : kernel;
                    }
                }
            }
        }
    }

    {
        std::lock_guard<std::mutex> lock(convertMutex);
        std::shared_ptr<Image> image = std::make_shared<Image>(*camera->sharedImage());
        filter.initialize(image->width(), image->height());
        if(hue > 0.0 || saturation > 0.0 || value > 0.0) {
            filter.hsv(image.get(), hue, saturation, value);
        }
        if(red > 0.0 || green > 0.0 || blue > 0.0) {
            filter.rgb(image.get(), red, green, blue);
        }
        if(std_dev > 0.0) {
            filter.gaussian_noise(image.get(), std_dev);
        }
        if(salt_chance > 0.0) {
            if(salt_amount > 0.0) {
                filter.random_salt(image.get(), salt_amount, salt_chance);
            }
        }
        if(pepper_chance > 0.0) {
            if(pepper_amount > 0.0) {
                filter.random_pepper(image.get(), pepper_amount, pepper_chance);
            }
        }
        if(coef_b < 0.0 || coef_d > 1.0) {
            filter.barrel_distortion(image.get(), coef_b, coef_d);
        }
        if(mosaic_chance > 0.0) {
            filter.random_mosaic(image.get(), mosaic_chance, kernel);
        }
        camera->setImage(image);
    }
}


Item* VFXVisionSimulatorItem::doCloneItem(CloneMap* cloneMap) const
{
    return new VFXVisionSimulatorItem(*this);
}


void VFXVisionSimulatorItem::doPutProperties(PutPropertyFunction& putProperty)
{
    GLVisionSimulatorItem::doPutProperties(putProperty);
    putProperty(_("VFX event file"), FilePathProperty(impl->vfx_event_file_path),
                [this](const std::string& value){
                    impl->vfx_event_file_path = value;
                    return true;
                });
}


bool VFXVisionSimulatorItem::store(Archive& archive)
{
    if(!GLVisionSimulatorItem::store(archive)) {
        return false;
    }
    archive.writeRelocatablePath("vfx_event_file_path", impl->vfx_event_file_path);
    return true;
}


bool VFXVisionSimulatorItem::restore(const Archive& archive)
{
    if(!GLVisionSimulatorItem::restore(archive)) {
        return false;
    }
    string symbol;
    if(archive.read("vfx_event_file_path", symbol)) {
        symbol = archive.resolveRelocatablePath(symbol);
        if(!symbol.empty()) {
            impl->vfx_event_file_path = symbol;
        }
    }
    return true;
}