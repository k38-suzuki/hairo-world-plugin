/**
   \file
   \author Kenta Suzuki
*/

#include "PassiveMarker.h"
#include <cnoid/EigenUtil>
#include <cnoid/MeshGenerator>
#include <cnoid/SceneDevice>
#include <cnoid/SceneMarkers>
#include <cnoid/SceneShape>
#include <cnoid/YAMLBodyLoader>
#include <cnoid/YAMLReader>

using namespace cnoid;

bool readPassiveMarker(YAMLBodyLoader& loader, Mapping& node)
{
    PassiveMarkerPtr marker = new PassiveMarker;
    double t;
    if(node.read("radius", t)) marker->setRadius(t);
    auto& c = *node.findListing("color");
    if(c.size() == 3) {
        Vector3 color(c[0].toDouble(), c[1].toDouble(), c[2].toDouble());
        marker->setColor(color);
    }
    if(node.read("transparency", t)) marker->setTransparency(t);
    return loader.readDevice(marker, node);
}


namespace cnoid {

class ScenePassiveMarker : public SceneDevice
{
public:
    ScenePassiveMarker(Device* device);

private:
    void updateScene();
    PassiveMarker* passiveMarker;
    SgShape* shape;
    bool isPassiveMarkerAttached;
};


ScenePassiveMarker::ScenePassiveMarker(Device* device)
    :SceneDevice(device)
{
    MeshGenerator generator;
    passiveMarker = static_cast<PassiveMarker*>(device);
    shape = new SgShape();
    shape->setMesh(generator.generateSphere(passiveMarker->radius()));
    shape->setName(device->name());
    SgMaterial* material = new SgMaterial();
    material->setDiffuseColor(passiveMarker->color());
    material->setTransparency(passiveMarker->transparency());
    shape->setMaterial(material);
    isPassiveMarkerAttached = false;
    setFunctionOnStateChanged([&](){ updateScene(); });
}


void ScenePassiveMarker::updateScene()
{
    bool on = passiveMarker->on();
    if(on != isPassiveMarkerAttached) {
        if(on) {
            addChildOnce(shape);
        }
        else {
            removeChild(shape);
        }
        isPassiveMarkerAttached = on;
    }

    MeshGenerator generator;
    shape->setMesh(generator.generateSphere(passiveMarker->radius()));
    SgMaterial* material = new SgMaterial();
    material->setDiffuseColor(passiveMarker->color());
    material->setTransparency(passiveMarker->transparency());
    shape->setMaterial(material);
    shape->notifyUpdate();
}

}

SceneDevice* createScenePassiveMarker(Device* device)
{
    return new ScenePassiveMarker(device);
}


struct PassiveMarkerRegistration
{
    PassiveMarkerRegistration() {
        YAMLBodyLoader::addNodeType("PassiveMarker", readPassiveMarker);
        SceneDevice::registerSceneDeviceFactory<PassiveMarker>(createScenePassiveMarker);
    }
} registrationPassiveMarker;


PassiveMarker::PassiveMarker()
{
    on_ = true;
    radius_ = 1.0;
    color_ << 1.0, 0.0, 0.0;
    transparency_ = 0.0;
}


PassiveMarker::PassiveMarker(const PassiveMarker &org, bool copyStateOnly)
    : Device(org, copyStateOnly)
{
    copyStateFrom(org);
}


const char* PassiveMarker::typeName()
{
    return "PassiveMarker";
}


void PassiveMarker::copyStateFrom(const PassiveMarker& other)
{
    on_ = other.on_;
    radius_ = other.radius_;
    color_ = other.color_;
    transparency_ = other.transparency_;
}


void PassiveMarker::copyStateFrom(const DeviceState& other)
{
    if(typeid(other) != typeid(PassiveMarker)){
        throw std::invalid_argument("Type mismatch in the Device::copyStateFrom function");
    }
    copyStateFrom(static_cast<const PassiveMarker&>(other));
}


DeviceState* PassiveMarker::cloneState() const
{
    return new PassiveMarker(*this, true);
}


Referenced* PassiveMarker::doClone(CloneMap*) const
{
    return new PassiveMarker(*this);
}


void PassiveMarker::forEachActualType(std::function<bool(const std::type_info& type)> func)
{
    if(!func(typeid(PassiveMarker))){
        Device::forEachActualType(func);
    }
}


void PassiveMarker::clearState()
{

}


bool PassiveMarker::on() const
{
    return on_;
}


void PassiveMarker::on(bool on)
{
    on_ = on;
}


int PassiveMarker::stateSize() const
{
    return 6;
}


const double* PassiveMarker::readState(const double* buf)
{
    int i = 0;
    on_ = buf[i++];
    radius_ = buf[i++];
    color_[0] = buf[i++];
    color_[1] = buf[i++];
    color_[2] = buf[i++];
    transparency_ = buf[i++];
    return buf + i;
}


double* PassiveMarker::writeState(double* out_buf) const
{
    int i = 0;
    out_buf[i++] = on_ ? 1.0 : 0.0;
    out_buf[i++] = radius_;
    out_buf[i++] = color_[0];
    out_buf[i++] = color_[1];
    out_buf[i++] = color_[2];
    out_buf[i++] = transparency_;
    return out_buf + i;
}
