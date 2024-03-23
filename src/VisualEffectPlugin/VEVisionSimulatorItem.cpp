/**
   @author Kenta Suzuki
*/

#include "VEVisionSimulatorItem.h"
#include <cnoid/Body>
#include <cnoid/DeviceList>
#include <cnoid/ItemManager>
#include <cnoid/SimulatorItem>
#include <cnoid/WorldItem>
#include "CameraEffect.h"
#include "ImageGenerator.h"
#include "NoisyCamera.h"
#include "VEAreaItem.h"
#include "gettext.h"

using namespace std;
using namespace cnoid;

namespace cnoid {

class VEVisionSimulatorItem::Impl
{
public:
    VEVisionSimulatorItem* self;

    Impl(VEVisionSimulatorItem* self);
    Impl(VEVisionSimulatorItem* self, const Impl& org);
    ~Impl();

    DeviceList<Camera> cameras;
    DeviceList<NoisyCamera> noisyCameras;
    ItemList<VEAreaItem> areaItems;

    ImageGenerator generator;

    bool initializeSimulation(SimulatorItem* simulatorItem);
    void onPostDynamics();
};

}


void VEVisionSimulatorItem::initializeClass(ExtensionManager* ext)
{
    ItemManager& im = ext->itemManager();
    im.registerClass<VEVisionSimulatorItem, SubSimulatorItem>(N_("VEVisionSimulatorItem"));
    im.addCreationPanel<VEVisionSimulatorItem>();
}


VEVisionSimulatorItem::VEVisionSimulatorItem()
    : GLVisionSimulatorItem()
{
    impl = new Impl(this);
}

VEVisionSimulatorItem::Impl::Impl(VEVisionSimulatorItem* self)
    : self(self)
{
    cameras.clear();
    noisyCameras.clear();
    areaItems.clear();
}


VEVisionSimulatorItem::VEVisionSimulatorItem(const VEVisionSimulatorItem& org)
    : GLVisionSimulatorItem(org),
      impl(new Impl(this, *org.impl))
{

}


VEVisionSimulatorItem::Impl::Impl(VEVisionSimulatorItem* self, const Impl& org)
    : self(self)
{
    cameras.clear();
    noisyCameras.clear();
    areaItems.clear();
}


VEVisionSimulatorItem::~VEVisionSimulatorItem()
{

}


bool VEVisionSimulatorItem::initializeSimulation(SimulatorItem* simulatorItem)
{
    if(!GLVisionSimulatorItem::initializeSimulation(simulatorItem)) {
        return false;
    }
    return impl->initializeSimulation(simulatorItem);
}


bool VEVisionSimulatorItem::Impl::initializeSimulation(SimulatorItem* simulatorItem)
{
    cameras.clear();
    noisyCameras.clear();
    areaItems.clear();

    const vector<SimulationBody*>& simBodies = simulatorItem->simulationBodies();
    for(auto& simBody : simBodies) {
        Body* body = simBody->body();
        cameras << body->devices();
        noisyCameras << body->devices();
    }

    WorldItem* worldItem = simulatorItem->findOwnerItem<WorldItem>();
    if(worldItem) {
        areaItems = worldItem->descendantItems<VEAreaItem>();
    }

    if(cameras.size() || noisyCameras.size()) {
        simulatorItem->addPostDynamicsFunction([&](){ onPostDynamics(); });
    }

    return true;
}


void VEVisionSimulatorItem::Impl::onPostDynamics()
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
        bool flipped = camera->flip();
        CameraEffect::FilterType filterType = camera->filterType();

        for(auto& areaItem : areaItems) {
            bool is_collided = areaItem->isCollided(link->T().translation());
            if(is_collided) {
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
                filterType = (CameraEffect::FilterType)areaItem->filter();
            }
        }

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
        if(filterType == CameraEffect::GAUSSIAN_3X3) {
            generator.gaussianFilter(image, 3);
        } else if(filterType == CameraEffect::GAUSSIAN_5X5) {
            generator.gaussianFilter(image, 5);
        } else if(filterType == CameraEffect::SOBEL) {
            generator.sobelFilter(image);
        } else if(filterType == CameraEffect::PREWITT) {
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
        CameraEffect::FilterType filterType = CameraEffect::NO_FILTER;

        for(auto& areaItem : areaItems) {
            bool is_collided = areaItem->isCollided(link->T().translation());
            if(is_collided) {
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
                filterType = (CameraEffect::FilterType)areaItem->filter();
            }
        }

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
        if(filterType == CameraEffect::GAUSSIAN_3X3) {
            generator.gaussianFilter(image, 3);
        } else if(filterType == CameraEffect::GAUSSIAN_5X5) {
            generator.gaussianFilter(image, 5);
        } else if(filterType == CameraEffect::SOBEL) {
            generator.sobelFilter(image);
        } else if(filterType == CameraEffect::PREWITT) {
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


Item* VEVisionSimulatorItem::doCloneItem(CloneMap* cloneMap) const
{
    return new VEVisionSimulatorItem(*this);
}


void VEVisionSimulatorItem::doPutProperties(PutPropertyFunction& putProperty)
{
    GLVisionSimulatorItem::doPutProperties(putProperty);
}


bool VEVisionSimulatorItem::store(Archive& archive)
{
    if(!GLVisionSimulatorItem::store(archive)) {
        return false;
    }
    return true;
}


bool VEVisionSimulatorItem::restore(const Archive& archive)
{
    if(!GLVisionSimulatorItem::restore(archive)) {
        return false;
    }
    return true;
}
