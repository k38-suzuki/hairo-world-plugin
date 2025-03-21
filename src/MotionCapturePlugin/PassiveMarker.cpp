/**
   @author Kenta Suzuki
*/

#include "PassiveMarker.h"
#include <cnoid/EigenArchive>
#include <cnoid/EigenUtil>
#include <cnoid/MeshGenerator>
#include <cnoid/SceneDevice>
#include <cnoid/SceneMarkers>
#include <cnoid/SceneShape>
#include <cnoid/StdBodyLoader>
#include <cnoid/StdBodyWriter>
#include <cnoid/YAMLReader>

using namespace cnoid;

namespace cnoid {

class ScenePassiveMarker : public SceneDevice
{
public:
    ScenePassiveMarker(Device* device);

private:
    void onStateChanged();
    PassiveMarker* passiveMarker;
    SgPosTransformPtr scene;
    bool isPassiveMarkerAttached;
};


ScenePassiveMarker::ScenePassiveMarker(Device* device)
    :SceneDevice(device)
{
    MeshGenerator generator;
    passiveMarker = static_cast<PassiveMarker*>(device);
    if(!scene) {
        scene = new SgPosTransform;
        SgShape* shape = new SgShape;
        shape->setMesh(generator.generateSphere(passiveMarker->radius()));
        shape->setName(device->name());
        SgMaterial* material = shape->getOrCreateMaterial();
        material->setDiffuseColor(passiveMarker->color());
        material->setTransparency(passiveMarker->transparency());
        scene->addChild(shape);
    }
    isPassiveMarkerAttached = false;
    setFunctionOnStateChanged([&](){ onStateChanged(); });
}


void ScenePassiveMarker::onStateChanged()
{
    bool on = passiveMarker->on();
    bool symbol = passiveMarker->symbol();
    if(on != isPassiveMarkerAttached && symbol) {
        if(on) {
            addChildOnce(scene);
        } else {
            removeChild(scene);
        }
        isPassiveMarkerAttached = on;
    }

    SgShape* shape = dynamic_cast<SgShape*>(scene->child(0));
    if(shape) {
        SgMaterial* material = shape->getOrCreateMaterial();
        material->setDiffuseColor(passiveMarker->color());
        material->setTransparency(passiveMarker->transparency());
        shape->notifyUpdate();
    }
}

}


SceneDevice* createScenePassiveMarker(Device* device)
{
    return new ScenePassiveMarker(device);
}


PassiveMarker::PassiveMarker()
    : Device()
{
    on_ = true;
    radius_ = 0.05;
    color_ << 1.0, 0.0, 0.0;
    transparency_ = 0.0;
    symbol_ = true;
}


PassiveMarker::PassiveMarker(const PassiveMarker &org, bool copyStateOnly)
    : Device(org, copyStateOnly)
{
    copyStateFrom(org);
}


const char* PassiveMarker::typeName() const
{
    return "PassiveMarker";
}


void PassiveMarker::copyStateFrom(const PassiveMarker& other)
{
    on_ = other.on_;
    radius_ = other.radius_;
    color_ = other.color_;
    transparency_ = other.transparency_;
    symbol_ = other.symbol_;
}


void PassiveMarker::copyStateFrom(const DeviceState& other)
{
    if(typeid(other) != typeid(PassiveMarker)) {
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
    if(!func(typeid(PassiveMarker))) {
        Device::forEachActualType(func);
    }
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
    symbol_ = buf[i++];
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
    out_buf[i++] = symbol_ ? 1.0 : 0.0;
    return out_buf + i;
}


bool PassiveMarker::readSpecifications(const Mapping* info)
{
    info->read("on", on_);
    info->read("radius", radius_);
    read(info, "color", color_);
    info->read("transparency", transparency_);
    info->read("symbol", symbol_);
    return true;
}


bool PassiveMarker::writeSpecifications(Mapping* info) const
{
    info->write("on", on_);
    info->write("radius", radius_);
    write(info, "color", color_);
    info->write("transparency", transparency_);
    info->write("symbol", symbol_);
    return true;
}


namespace {

bool readPassiveMarker(StdBodyLoader* loader, const Mapping* info)
{
    PassiveMarkerPtr marker = new PassiveMarker;
    if(!marker->readSpecifications(info)) {
            marker.reset();
    }

    bool result = false;
    if(marker) {
        result = loader->readDevice(marker, info);
    }
    return result;
}


struct PassiveMarkerRegistration
{
    PassiveMarkerRegistration() {
        StdBodyLoader::registerNodeType("PassiveMarker", readPassiveMarker);
        StdBodyWriter::registerDeviceWriter<PassiveMarker>(
                    "PassiveMarker",
                    [](StdBodyWriter* /* writer */, Mapping* info, const PassiveMarker* marker) {
            return marker->writeSpecifications(info);
        });
        SceneDevice::registerSceneDeviceFactory<PassiveMarker>(createScenePassiveMarker);
    }
} registration;

}