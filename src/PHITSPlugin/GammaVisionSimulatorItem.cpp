/**
   @author Kenta Suzuki
*/

#include "GammaVisionSimulatorItem.h"
#include <cnoid/Body>
#include <cnoid/DeviceList>
#include <cnoid/ItemManager>
#include <cnoid/SimulatorItem>
#include <cnoid/WorldItem>
#include "ComptonCamera.h"
#include "GammaEffect.h"
#include "GammaImageGenerator.h"
#include "PinholeCamera.h"
#include "gettext.h"

using namespace std;
using namespace cnoid;

namespace cnoid {

class GammaVisionSimulatorItem::Impl
{
public:
    GammaVisionSimulatorItem* self;

    Impl(GammaVisionSimulatorItem* self);
    Impl(GammaVisionSimulatorItem* self, const Impl& org);
    ~Impl();

    DeviceList<ComptonCamera> comptonCameras;
    DeviceList<PinholeCamera> pinholeCameras;

    GammaImageGenerator generator;
    vector<GammaEffect*> comptonEffects;
    vector<GammaEffect*> pinholeEffects;

    bool initializeSimulation(SimulatorItem* simulatorItem);
    void finalizeSimulation();
    void onPostDynamics();
};

}


void GammaVisionSimulatorItem::initializeClass(ExtensionManager* ext)
{
    ItemManager& im = ext->itemManager();
    im.registerClass<GammaVisionSimulatorItem, SubSimulatorItem>(N_("GammaVisionSimulatorItem"));
    im.addCreationPanel<GammaVisionSimulatorItem>();
}


GammaVisionSimulatorItem::GammaVisionSimulatorItem()
    : GLVisionSimulatorItem()
{
    impl = new Impl(this);
}

GammaVisionSimulatorItem::Impl::Impl(GammaVisionSimulatorItem* self)
    : self(self)
{
    comptonCameras.clear();
    pinholeCameras.clear();
    comptonEffects.clear();
    pinholeEffects.clear();
}


GammaVisionSimulatorItem::GammaVisionSimulatorItem(const GammaVisionSimulatorItem& org)
    : GLVisionSimulatorItem(org),
      impl(new Impl(this, *org.impl))
{

}


GammaVisionSimulatorItem::Impl::Impl(GammaVisionSimulatorItem* self, const Impl& org)
    : self(self)
{
    comptonCameras.clear();
    pinholeCameras.clear();
    comptonEffects.clear();
    pinholeEffects.clear();
}


GammaVisionSimulatorItem::~GammaVisionSimulatorItem()
{
    delete impl;
}


bool GammaVisionSimulatorItem::initializeSimulation(SimulatorItem* simulatorItem)
{
    if(!GLVisionSimulatorItem::initializeSimulation(simulatorItem)) {
        return false;
    }
    return impl->initializeSimulation(simulatorItem);
}


bool GammaVisionSimulatorItem::Impl::initializeSimulation(SimulatorItem* simulatorItem)
{
    comptonCameras.clear();
    pinholeCameras.clear();
    comptonEffects.clear();
    pinholeEffects.clear();

    const vector<SimulationBody*>& simBodies = simulatorItem->simulationBodies();
    for(auto& simBody : simBodies) {
        Body* body = simBody->body();
        comptonCameras << body->devices();
        pinholeCameras << body->devices();
    }

    for(auto& camera : comptonCameras) {
        GammaEffect* effect = new GammaEffect(camera);
        effect->start(true);
        comptonEffects.push_back(effect);
    }

    for(auto& camera : pinholeCameras) {
        GammaEffect* effect = new GammaEffect(camera);
        effect->start(true);
        pinholeEffects.push_back(effect);
    }

    if(comptonCameras.size() || pinholeCameras.size()) {
        simulatorItem->addPostDynamicsFunction([&](){ onPostDynamics(); });
    }

    return true;
}


void GammaVisionSimulatorItem::finalizeSimulation()
{
    impl->finalizeSimulation();
}


void GammaVisionSimulatorItem::Impl::finalizeSimulation()
{
    for(auto& effect : comptonEffects) {
        effect->start(false);
    }

    for(auto& effect : pinholeEffects) {
        effect->start(false);
    }
}


void GammaVisionSimulatorItem::Impl::onPostDynamics()
{
    for(auto& camera : comptonCameras) {
        Image image = *camera->sharedImage();
        if(!image.empty()) {
            std::shared_ptr<Image> sharedImage = std::make_shared<Image>(image);
            generator.generateImage(camera, sharedImage);
            camera->setImage(sharedImage);
        }
    }

    for(auto& camera : pinholeCameras) {
        Image image = *camera->sharedImage();
        if(!image.empty()) {
            std::shared_ptr<Image> sharedImage = std::make_shared<Image>(image);
            generator.generateImage(camera, sharedImage);
            camera->setImage(sharedImage);
        }
    }
}


Item* GammaVisionSimulatorItem::doCloneItem(CloneMap* cloneMap) const
{
    return new GammaVisionSimulatorItem(*this);
}


void GammaVisionSimulatorItem::doPutProperties(PutPropertyFunction& putProperty)
{
    GLVisionSimulatorItem::doPutProperties(putProperty);
}


bool GammaVisionSimulatorItem::store(Archive& archive)
{
    if(!GLVisionSimulatorItem::store(archive)) {
        return false;
    }
    return true;
}


bool GammaVisionSimulatorItem::restore(const Archive& archive)
{
    if(!GLVisionSimulatorItem::restore(archive)) {
        return false;
    }
    return true;
}
