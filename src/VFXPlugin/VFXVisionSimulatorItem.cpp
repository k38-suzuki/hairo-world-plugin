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

    bool initializeSimulation(SimulatorItem* simulatorItem);
    void onPostDynamics();
    void onPostDynamics2();
    void onPostDynamics3();
    bool setDynamicsType(int dynamicsId);
    double dynamicsType() const;

    DeviceList<Camera> cameras;
    ItemList<MultiColliderItem> colliders;
    enum DynamicsId { RANDOM_SALT_ONLY, RANDOM_MOSAIC_ONLY, ALL_PROCESS };
    Selection dynamicsSelection;
    std::mutex convertMutex;
    VFXConverter converter;
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
    dynamicsSelection.setSymbol(RANDOM_SALT_ONLY, N_("Random salt only"));
    dynamicsSelection.setSymbol(RANDOM_MOSAIC_ONLY, N_("Random mosaic only"));
    dynamicsSelection.setSymbol(ALL_PROCESS, N_("All process"));
    dynamicsSelection.select(RANDOM_SALT_ONLY);
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
        case RANDOM_SALT_ONLY:
                simulatorItem->addPostDynamicsFunction([&](){ onPostDynamics(); });
            break;
        case RANDOM_MOSAIC_ONLY:
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
        double salt_amount = 0.0;
        double salt_chance = 1.0;

        NoisyCamera* noisyCamera = dynamic_cast<NoisyCamera*>(camera.get());
        if(noisyCamera) {
            salt_amount = noisyCamera->saltAmount();
            salt_chance = noisyCamera->saltChance();
        }

        for(auto& collider : colliders) {
            if(collision(collider, link->T().translation())) {
                salt_amount = collider->saltAmount();
                salt_chance = collider->saltChance();
            }
        }

        {
            std::lock_guard<std::mutex> lock(convertMutex);
            std::shared_ptr<Image> image = std::make_shared<Image>(*camera->sharedImage());
            converter.initialize(image->width(), image->height());
            if(salt_chance > 0.0) {
                if(salt_amount > 0.0) {
                    converter.random_salt(image.get(), salt_amount, salt_chance);
                }
            }
            camera->setImage(image);
        }
    }
}


void VFXVisionSimulatorItem::Impl::onPostDynamics2()
{
    for(auto& camera : cameras) {
        Link* link = camera->link();
        double mosaic_chance = 1.0;
        int kernel = 16;

        NoisyCamera* noisyCamera = dynamic_cast<NoisyCamera*>(camera.get());
        if(noisyCamera) {
            mosaic_chance = noisyCamera->mosaicChance();
            kernel = noisyCamera->kernel();
        }

        for(auto& collider : colliders) {
            if(collision(collider, link->T().translation())) {
                mosaic_chance = collider->mosaicChance();
                kernel = collider->kernel();
            }
        }

        {
            std::lock_guard<std::mutex> lock(convertMutex);
            std::shared_ptr<Image> image = std::make_shared<Image>(*camera->sharedImage());
            converter.initialize(image->width(), image->height());
            if(mosaic_chance > 0.0) {
                converter.random_mosaic(image.get(), mosaic_chance, kernel);
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
        double salt_amount = 0.0;
        double salt_chance = 1.0;
        double pepper_amount = 0.0;
        double pepper_chance = 1.0;
        double mosaic_chance = 1.0;
        int kernel = 16;

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
            salt_amount = noisyCamera->saltAmount();
            salt_chance = noisyCamera->saltChance();
            pepper_amount = noisyCamera->pepperAmount();
            pepper_chance = noisyCamera->pepperChance();
            mosaic_chance = noisyCamera->mosaicChance();
            kernel = noisyCamera->kernel();
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
                salt_amount = collider->saltAmount();
                salt_chance = collider->saltChance();
                pepper_amount = collider->pepperAmount();
                pepper_chance = collider->pepperChance();
                mosaic_chance = collider->mosaicChance();
                kernel = collider->kernel();
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
            if(salt_chance > 0.0) {
                if(salt_amount > 0.0) {
                    converter.random_salt(image.get(), salt_amount, salt_chance);
                }
            }
            if(pepper_chance > 0.0) {
                if(pepper_amount > 0.0) {
                    converter.random_pepper(image.get(), pepper_amount, pepper_chance);
                }
            }
            if(coef_b < 0.0 || coef_d > 1.0) {
                converter.barrel_distortion(image.get(), coef_b, coef_d);
            }
            if(mosaic_chance > 0.0) {
                converter.random_mosaic(image.get(), mosaic_chance, kernel);
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
}


bool VFXVisionSimulatorItem::store(Archive& archive)
{
    if(!GLVisionSimulatorItem::store(archive)) {
        return false;
    }
    archive.write("dynamics_type", impl->dynamicsSelection.selectedSymbol());
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
    return true;
}
