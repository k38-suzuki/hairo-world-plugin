/**
   \file
   \author Kenta Suzuki
*/

#include "Thruster.h"
#include <cnoid/Archive>
#include <cnoid/EigenUtil>
#include <cnoid/MeshGenerator>
#include <cnoid/Link>
#include <cnoid/SceneDevice>
#include <cnoid/SceneMarkers>
#include <cnoid/SceneShape>
#include <cnoid/StdBodyLoader>
#include <cnoid/StdBodyWriter>

using namespace cnoid;

namespace cnoid {

class SceneThruster : public SceneDevice
{
public:
    SceneThruster(Device* device);

private:
    void onStateChanged();
    Thruster* thrusterDevice;
    SgPosTransformPtr scene;
    bool isThrusterAttached;
};

}


SceneThruster::SceneThruster(Device* device)
    : SceneDevice(device)
{
    MeshGenerator generator;
    thrusterDevice = static_cast<Thruster*>(device);
    if(!scene) {
        scene = new SgPosTransform;
        SgShape* shape = new SgShape;
        shape->setMesh(generator.generateArrow(0.01, 0.05, 0.02, 0.05));
        shape->setName(device->name());
        SgMaterial* material = shape->getOrCreateMaterial();
        material->setDiffuseColor(Vector3(1.0, 0.0, 0.0));
        material->setTransparency(0.5);
        scene->addChild(shape);
        scene->setTranslation(Vector3(0.025, 0.0, 0.0));
        scene->setRotation(rotFromRpy(radian(Vector3(0.0, 0.0, -90.0))));
    }
    isThrusterAttached = false;
    setFunctionOnStateChanged([&](){ onStateChanged(); });
}


void SceneThruster::onStateChanged()
{
    bool on = thrusterDevice->on();
    bool symbol = thrusterDevice->symbol();
    if(on != isThrusterAttached && symbol) {
        if(on) {
            addChildOnce(scene);
        } else {
            removeChild(scene);
        }
        isThrusterAttached = on;
    }
}


SceneDevice* createSceneThruster(Device* device)
{
    return new SceneThruster(device);
}


Thruster::Thruster()
{
    on_ = true;
    force_ = 0.0;
    torque_ = 0.0;
    direction_ << 1.0, 0.0, 0.0;
    forceOffset_ = 0.0;
    torqueOffset_ = 0.0;
    symbol_ = true;
}


Thruster::Thruster(const Thruster& org, bool copyStateOnly)
    : Device(org, copyStateOnly)
{
    copyThrusterStateFrom(org);
}


const char* Thruster::typeName() const
{
    return "Thruster";
}


void Thruster::copyStateFrom(const Thruster& other)
{
    copyThrusterStateFrom(other);
}


void Thruster::copyStateFrom(const DeviceState& other)
{
    if(typeid(other) != typeid(Thruster)) {
        throw std::invalid_argument("Type mismatch in the Device::copyStateFrom function");
    }
    copyStateFrom(static_cast<const Thruster&>(other));
}


void Thruster::copyThrusterStateFrom(const Thruster& other)
{
    on_ = other.on_;
    force_ = other.force_;
    torque_ = other.torque_;
    direction_ = other.direction_;
    forceOffset_ = other.forceOffset_;
    torqueOffset_ = other.torqueOffset_;
    symbol_ = other.symbol_;
}


DeviceState* Thruster::cloneState() const
{
    return new Thruster(*this, true);
}


Referenced* Thruster::doClone(CloneMap*) const
{
    return new Thruster(*this);
}


void Thruster::forEachActualType(std::function<bool(const std::type_info& type)> func)
{
    if(!func(typeid(Thruster))) {
        Device::forEachActualType(func);
    }
}


void Thruster::clearState()
{
    force_ = 0.0;
    torque_ = 0.0;
}


bool Thruster::on() const
{
    return on_;
}


void Thruster::on(bool on)
{
    on_ = on;
}


int Thruster::stateSize() const
{
    return 9;
}


const double* Thruster::readState(const double* buf)
{
    int i = 0;
    on_ = buf[i++];
    force_ = buf[i++];
    torque_ = buf[i++];
    direction_[0] = buf[i++];
    direction_[1] = buf[i++];
    direction_[2] = buf[i++];
    forceOffset_ = buf[i++];
    torqueOffset_ = buf[i++];
    symbol_ = buf[i++];
    return buf + i;
}


double* Thruster::writeState(double* out_buf) const
{
    int i = 0;
    out_buf[i++] = on_ ? 1.0 : 0.0;
    out_buf[i++] = force_;
    out_buf[i++] = torque_;
    out_buf[i++] = direction_[0];
    out_buf[i++] = direction_[1];
    out_buf[i++] = direction_[2];
    out_buf[i++] = forceOffset_;
    out_buf[i++] = torqueOffset_;
    out_buf[i++] = symbol_ ? 1.0 : 0.0;
    return out_buf + i;
}


bool Thruster::readSpecifications(const Mapping* info)
{
    info->read("on", on_);
    info->read({ "force_offset", "forceOffset" }, forceOffset_);
    info->read({ "torque_offset", "torqueOffset" }, torqueOffset_);
    info->read("symbol", symbol_);

    return true;
}


bool Thruster::writeSpecifications(Mapping* info) const
{
    info->write("on", on_);
    info->write("force_offset", forceOffset_);
    info->write("torque_offset", torqueOffset_);
    info->write("symbol", symbol_);

    return true;
}


namespace {

bool readThruster(StdBodyLoader* loader, const Mapping* info)
{
    ThrusterPtr thruster = new Thruster;
    if(!thruster->readSpecifications(info)) {
        thruster.reset();
    }

    bool result = false;
    if(thruster) {
        result = loader->readDevice(thruster, info);
    }
    return result;
}


struct ThrusterRegistration
{
    ThrusterRegistration() {
        StdBodyLoader::registerNodeType("Thruster", readThruster);
        StdBodyWriter::registerDeviceWriter<Thruster>(
                    "Thruster",
                    [](StdBodyWriter* /* writer */, Mapping* info, const Thruster* thruster) {
            return thruster->writeSpecifications(info);
        });
        SceneDevice::registerSceneDeviceFactory<Thruster>(createSceneThruster);
    }
} registrationThruster;

}
