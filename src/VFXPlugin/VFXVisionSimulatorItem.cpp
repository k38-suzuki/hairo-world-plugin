/**
   @author Kenta Suzuki
*/

#include "VFXVisionSimulatorItem.h"
#include <cnoid/Body>
#include <cnoid/DeviceList>
#include <cnoid/ItemManager>
#include <cnoid/SimulatorItem>
#include <cnoid/WorldItem>
#include <mutex>
#include "VisualEffects.h"
#include "ImageGenerator.h"
#include "NoisyCamera.h"
#include "VFXColliderItem.h"
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
    ItemList<VFXColliderItem> colliders;
    std::mutex convertMutex;

    ImageGenerator generator;

    bool initializeSimulation(SimulatorItem* simulatorItem);
    void onPostDynamics();
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
        colliders = worldItem->descendantItems<VFXColliderItem>();
    }

    if(cameras.size() || noisyCameras.size()) {
        simulatorItem->addPostDynamicsFunction([&](){ onPostDynamics(); });
    }

    return true;
}


void VFXVisionSimulatorItem::Impl::onPostDynamics()
{
    for(auto& camera : noisyCameras) {
        Link* link = camera->link();
        double hue = camera->hsv()[0];
        double saturation = camera->hsv()[1];
        double value = camera->hsv()[2];
        double red = camera->rgb()[0];
        double green = camera->rgb()[1];
        double blue = camera->rgb()[2];
        double coefB = camera->coefB();
        double coefD = camera->coefD();
        double stdDev = camera->stdDev();
        double salt = camera->salt();
        double pepper = camera->pepper();
        bool flipped = camera->flipped();
        VisualEffects::FilterType filterType = camera->filterType();

        for(auto& collider : colliders) {
            if(collision(collider, link->T().translation())) {
                hue = collider->hsv()[0];
                saturation = collider->hsv()[1];
                value = collider->hsv()[2];
                red = collider->rgb()[0];
                green = collider->rgb()[1];
                blue = collider->rgb()[2];
                coefB = collider->coefB();
                coefD = collider->coefD();
                stdDev = collider->stdDev();
                salt = collider->salt();
                pepper = collider->pepper();
                flipped = collider->flipped();
                filterType = (VisualEffects::FilterType)collider->filterType();
            }
        }

        {
            std::lock_guard<std::mutex> lock(convertMutex);
            Image image = *camera->sharedImage();

            // convert image
            if(hue > 0.0 || saturation > 0.0 || value > 0.0) {
                generator.hsv(image, hue, saturation, value);
            }
            if(red > 0.0 || green > 0.0 || blue > 0.0) {
                generator.rgb(image, red, green, blue);
            }
            if(flipped) {
                generator.flippedImage(image);
            }
            if(stdDev > 0.0) {
                generator.gaussianNoise(image, stdDev);
            }
            if(salt > 0.0 || pepper > 0.0) {
                generator.saltPepperNoise(image, salt, pepper);
            }
            if(filterType == VisualEffects::GAUSSIAN_3X3) {
                generator.gaussianFilter(image, 3);
            } else if(filterType == VisualEffects::GAUSSIAN_5X5) {
                generator.gaussianFilter(image, 5);
            } else if(filterType == VisualEffects::SOBEL) {
                generator.sobelFilter(image);
            } else if(filterType == VisualEffects::PREWITT) {
                generator.prewittFilter(image);
            }
            if(coefB < 0.0 || coefD > 1.0) {
                generator.barrelDistortion(image, coefB, coefD);
            }

            if(!image.empty()) {
                std::shared_ptr<Image> sharedImage = std::make_shared<Image>(image);
                camera->setImage(sharedImage);
            }
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
        double coefB = 0.0;
        double coefD = 0.0;
        double stdDev = 0.0;
        double salt = 0.0;
        double pepper = 0.0;
        bool flipped = false;
        VisualEffects::FilterType filterType = VisualEffects::NO_FILTER;

        for(auto& collider : colliders) {
            if(collision(collider, link->T().translation())) {
                hue = collider->hsv()[0];
                saturation = collider->hsv()[1];
                value = collider->hsv()[2];
                red = collider->rgb()[0];
                green = collider->rgb()[1];
                blue = collider->rgb()[2];
                coefB = collider->coefB();
                coefD = collider->coefD();
                stdDev = collider->stdDev();
                salt = collider->salt();
                pepper = collider->pepper();
                flipped = collider->flipped();
                filterType = (VisualEffects::FilterType)collider->filterType();
            }
        }

        {
            std::lock_guard<std::mutex> lock(convertMutex);
            Image image = *camera->sharedImage();

            // convert image
            if(hue > 0.0 || saturation > 0.0 || value > 0.0) {
                generator.hsv(image, hue, saturation, value);
            }
            if(red > 0.0 || green > 0.0 || blue > 0.0) {
                generator.rgb(image, red, green, blue);
            }
            if(flipped) {
                generator.flippedImage(image);
            }
            if(stdDev > 0.0) {
                generator.gaussianNoise(image, stdDev);
            }
            if(salt > 0.0 || pepper > 0.0) {
                generator.saltPepperNoise(image, salt, pepper);
            }
            if(filterType == VisualEffects::GAUSSIAN_3X3) {
                generator.gaussianFilter(image, 3);
            } else if(filterType == VisualEffects::GAUSSIAN_5X5) {
                generator.gaussianFilter(image, 5);
            } else if(filterType == VisualEffects::SOBEL) {
                generator.sobelFilter(image);
            } else if(filterType == VisualEffects::PREWITT) {
                generator.prewittFilter(image);
            }
            if(coefB < 0.0 || coefD > 1.0) {
                generator.barrelDistortion(image, coefB, coefD);
            }

            if(!image.empty()) {
                std::shared_ptr<Image> sharedImage = std::make_shared<Image>(image);
                camera->setImage(sharedImage);
            }
        }
    }
}


Item* VFXVisionSimulatorItem::doCloneItem(CloneMap* cloneMap) const
{
    return new VFXVisionSimulatorItem(*this);
}


void VFXVisionSimulatorItem::doPutProperties(PutPropertyFunction& putProperty)
{
    GLVisionSimulatorItem::doPutProperties(putProperty);
}


bool VFXVisionSimulatorItem::store(Archive& archive)
{
    if(!GLVisionSimulatorItem::store(archive)) {
        return false;
    }
    return true;
}


bool VFXVisionSimulatorItem::restore(const Archive& archive)
{
    if(!GLVisionSimulatorItem::restore(archive)) {
        return false;
    }
    return true;
}
