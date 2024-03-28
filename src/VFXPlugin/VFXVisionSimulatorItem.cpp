/**
   @author Kenta Suzuki
*/

#include "VFXVisionSimulatorItem.h"
#include <cnoid/Body>
#include <cnoid/ItemManager>
#include <cnoid/WorldItem>
#include <cnoid/Selection>
#include <cnoid/Archive>
#include <cnoid/PutPropertyFunction>
#include <cnoid/DeviceList>
#include <cnoid/SimulatorItem>
#include <cnoid/MultiColliderItem>
#include <mutex>
#include "VFXConverter.h"
#include "NoisyCamera.h"
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

    DeviceList<Camera> cameras;
    ItemList<MultiColliderItem> colliders;
    enum DynamicsId { SALT_ONLY, RANDOM_MOSAIC, ALL_PROCESS };
    Selection dynamicsSelection;
    std::mutex convertMutex;

    double mosaic_rate;
    int mosaic_kernel;

    VFXConverter converter;

    bool initializeSimulation(SimulatorItem* simulatorItem);
    void onPostDynamics();
    void onPostDynamics2();
    void onPostDynamics3();
    bool setDynamicsType(int dynamicsId);
    double dynamicsType() const;
};

}


void VFXVisionSimulatorItem::initializeClass(ExtensionManager* ext)
{
    ext->itemManager()
        .registerClass<VFXVisionSimulatorItem, SubSimulatorItem>(N_("VFXVisionSimulatorItem"))
        .addCreationPanel<VFXVisionSimulatorItem>();
}


VFXVisionSimulatorItem::VFXVisionSimulatorItem()
    : GLVisionSimulatorItem()
{
    impl = new Impl(this);
}

VFXVisionSimulatorItem::Impl::Impl(VFXVisionSimulatorItem* self)
    : self(self)
{
    cameras.clear();
    colliders.clear();
    dynamicsSelection.setSymbol(SALT_ONLY, N_("Salt only"));
    dynamicsSelection.setSymbol(RANDOM_MOSAIC, N_("Random mosaic"));
    dynamicsSelection.setSymbol(ALL_PROCESS, N_("All process"));
    dynamicsSelection.select(SALT_ONLY);
    mosaic_rate = 0.0;
    mosaic_kernel = 16;
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
    dynamicsSelection = org.dynamicsSelection;
    mosaic_rate = org.mosaic_rate;
    mosaic_kernel = org.mosaic_kernel;
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

    if(cameras.size() > 0) {
        switch(dynamicsSelection.which()) {
        case SALT_ONLY:
                simulatorItem->addPostDynamicsFunction([&](){ onPostDynamics(); });
            break;
        case RANDOM_MOSAIC:
                simulatorItem->addPostDynamicsFunction([&](){ onPostDynamics2(); });
            break;
        case ALL_PROCESS:
                simulatorItem->addPostDynamicsFunction([&](){ onPostDynamics3(); });
            break;
        default:
            break;
        }
    }

    return true;
}


void VFXVisionSimulatorItem::Impl::onPostDynamics()
{
    for(auto& camera : cameras) {
        Link* link = camera->link();
        double salt = 0.0;

        NoisyCamera* noisyCamera = dynamic_cast<NoisyCamera*>(camera.get());
        if(noisyCamera) {
            salt = noisyCamera->salt();
        }

        for(auto& collider : colliders) {
            if(collision(collider, link->T().translation())) {
                salt = collider->salt();
            }
        }

        {
            std::lock_guard<std::mutex> lock(convertMutex);
            std::shared_ptr<Image> image = std::make_shared<Image>(*camera->sharedImage());
            converter.initialize(image->width(), image->height());
            if(salt > 0.0) {
                converter.salt(image.get(), salt);
            }
            camera->setImage(image);
        }
    }
}


void VFXVisionSimulatorItem::Impl::onPostDynamics2()
{
    for(auto& camera : cameras) {
        Link* link = camera->link();

        for(auto& collider : colliders) {
            if(collision(collider, link->T().translation())) {

            }
        }

        {
            std::lock_guard<std::mutex> lock(convertMutex);
            std::shared_ptr<Image> image = std::make_shared<Image>(*camera->sharedImage());
            converter.initialize(image->width(), image->height());
            if(mosaic_rate > 0.0) {
                converter.random_mosaic(image.get(), mosaic_rate, mosaic_kernel);
            }
            camera->setImage(image);
        }
    }
}


void VFXVisionSimulatorItem::Impl::onPostDynamics3()
{
    for(auto& camera : cameras) {
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
        double salt = 0.0;
        double pepper = 0.0;

        NoisyCamera* noisyCamera = dynamic_cast<NoisyCamera*>(camera.get());
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
            salt = noisyCamera->salt();
            pepper = noisyCamera->pepper();
        }

        for(auto& collider : colliders) {
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
                salt = collider->salt();
                pepper = collider->pepper();
            }
        }

        {
            std::lock_guard<std::mutex> lock(convertMutex);
            std::shared_ptr<Image> image = std::make_shared<Image>(*camera->sharedImage());
            converter.initialize(image->width(), image->height());
            if(hue > 0.0 || saturation > 0.0 || value > 0.0) {
                converter.hsv(image.get(), hue, saturation, value);
            }
            if(red > 0.0 || green > 0.0 || blue > 0.0) {
                converter.rgb(image.get(), red, green, blue);
            }
            if(std_dev > 0.0) {
                converter.gaussian_noise(image.get(), std_dev);
            }
            if(salt > 0.0) {
                converter.salt(image.get(), salt);
            }
            if(pepper > 0.0) {
                converter.pepper(image.get(), pepper);
            }
            if(coef_b < 0.0 || coef_d > 1.0) {
                converter.barrel_distortion(image.get(), coef_b, coef_d);
            }
            camera->setImage(image);
        }
    }
}


bool VFXVisionSimulatorItem::Impl::setDynamicsType(int dynamicsId)
{
    if(!dynamicsSelection.select(dynamicsId)) {
        return false;
    }
    self->notifyUpdate();
    return true;
}


double VFXVisionSimulatorItem::Impl::dynamicsType() const
{
    return dynamicsSelection.which();
}


Item* VFXVisionSimulatorItem::doCloneItem(CloneMap* cloneMap) const
{
    return new VFXVisionSimulatorItem(*this);
}


void VFXVisionSimulatorItem::doPutProperties(PutPropertyFunction& putProperty)
{
    GLVisionSimulatorItem::doPutProperties(putProperty);
    putProperty(_("Dynamics type"), impl->dynamicsSelection,
                [this](int which){ return impl->setDynamicsType(which); });

    switch(impl->dynamicsSelection.which()) {
    case VFXVisionSimulatorItem::Impl::SALT_ONLY:
        break;
    case VFXVisionSimulatorItem::Impl::RANDOM_MOSAIC:
        putProperty.min(0.0).max(1.0)(_("mosaic rate"),impl->mosaic_rate , changeProperty(impl->mosaic_rate));
        putProperty.min(8).max(64)(_("mosaic kernel"),impl->mosaic_kernel , changeProperty(impl->mosaic_kernel));    
        break;
    case VFXVisionSimulatorItem::Impl::ALL_PROCESS:
        break;
    }
}


bool VFXVisionSimulatorItem::store(Archive& archive)
{
    if(!GLVisionSimulatorItem::store(archive)) {
        return false;
    }
    archive.write("dynamics_type", impl->dynamicsSelection.selectedSymbol());
    archive.write("mosaic_rate", impl->mosaic_rate);
    archive.write("mosaic_kernel", impl->mosaic_kernel);
    return true;
}


bool VFXVisionSimulatorItem::restore(const Archive& archive)
{
    if(!GLVisionSimulatorItem::restore(archive)) {
        return false;
    }
    string dynamicsId;
    if(archive.read("dynamics_type", dynamicsId)) {
        impl->dynamicsSelection.select(dynamicsId);
    }
    archive.read("mosaic_rate", impl->mosaic_rate);
    archive.read("mosaic_kernel", impl->mosaic_kernel);
    return true;
}
