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
    DeviceList<NoisyCamera> noisyCameras;
    ItemList<MultiColliderItem> colliders;
    enum DynamicsId { SALT_ONLY, ALL_PROCESS };
    Selection dynamicsSelection;
    std::mutex convertMutex;

    VFXConverter converter;

    bool initializeSimulation(SimulatorItem* simulatorItem);
    void onPostDynamics();
    void onPostDynamics2();
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
    noisyCameras.clear();
    colliders.clear();
    dynamicsSelection.setSymbol(SALT_ONLY, N_("Salt only"));
    dynamicsSelection.setSymbol(ALL_PROCESS, N_("All process"));
    dynamicsSelection.select(SALT_ONLY);
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
    noisyCameras.clear();
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
    noisyCameras.clear();
    colliders.clear();

    const vector<SimulationBody*>& simBodies = simulatorItem->simulationBodies();
    for(auto& simBody : simBodies) {
        Body* body = simBody->body();
        cameras << body->devices();
        noisyCameras << body->devices();
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

    if(cameras.size() || noisyCameras.size()) {
        switch(dynamicsSelection.which()) {
        case SALT_ONLY:
                simulatorItem->addPostDynamicsFunction([&](){ onPostDynamics(); });
            break;
        case ALL_PROCESS:
                simulatorItem->addPostDynamicsFunction([&](){ onPostDynamics2(); });
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
            converter.salt(image.get(), salt);
            converter.pepper(image.get(), pepper);
            camera->setImage(image);
        }
    }
}


void VFXVisionSimulatorItem::Impl::onPostDynamics2()
{
    for(auto& camera : noisyCameras) {
        Link* link = camera->link();
        double hue = camera->hsv()[0];
        double saturation = camera->hsv()[1];
        double value = camera->hsv()[2];
        double red = camera->rgb()[0];
        double green = camera->rgb()[1];
        double blue = camera->rgb()[2];
        double coef_b = camera->coefB();
        double coef_d = camera->coefD();
        double std_dev = camera->stdDev();
        double salt = camera->salt();
        double pepper = camera->pepper();

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
            converter.hsv(image.get(), hue, saturation, value);
            converter.rgb(image.get(), red, green, blue);
            converter.gaussian_noise(image.get(), std_dev);
            converter.salt(image.get(), salt);
            converter.pepper(image.get(), pepper);
            converter.barrel_distortion(image.get(), coef_b, coef_d);
            camera->setImage(image);
        }
    }

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
            converter.hsv(image.get(), hue, saturation, value);
            converter.rgb(image.get(), red, green, blue);
            converter.gaussian_noise(image.get(), std_dev);
            converter.salt(image.get(), salt);
            converter.pepper(image.get(), pepper);
            converter.barrel_distortion(image.get(), coef_b, coef_d);
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
