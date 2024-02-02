/**
   @author Kenta Suzuki
*/

#include "CollisionSensor.h"
#include <cnoid/BoundingBox>
#include <cnoid/EigenArchive>
#include <cnoid/Link>
#include <cnoid/MeshGenerator>
#include <cnoid/SceneDevice>
#include <cnoid/StdBodyFileUtil>
#include <cnoid/ValueTree>

using namespace cnoid;
using namespace std;

namespace cnoid {

class SceneCollisionSensor : public SceneDevice
{
public:
    SceneCollisionSensor(Device* device);

private:
    void onStateChanged();
    CollisionSensor* sensorDevice;
    SgPosTransformPtr scene;
    bool isSensorAttached;
};

}


SceneCollisionSensor::SceneCollisionSensor(Device* device)
    : SceneDevice(device)
{
    MeshGenerator generator;
    sensorDevice = static_cast<CollisionSensor*>(device);
    if(!scene) {
        scene = new SgPosTransform;
        Link* link = sensorDevice->link();
        SgShape* shape = new SgShape;
        const BoundingBox& bb = link->visualShape()->boundingBox();
        shape->setMesh(generator.generateBox(bb.size() + Vector3(0.01, 0.01, 0.01)));
        shape->setName(sensorDevice->name());
        SgMaterial* material = shape->getOrCreateMaterial();
        material->setDiffuseColor(sensorDevice->color());
        material->setTransparency(0.5);
        scene->setTranslation(bb.center());
        scene->addChild(shape);
    }
    isSensorAttached = false;
    setFunctionOnStateChanged([&](){ onStateChanged(); });
}


void SceneCollisionSensor::onStateChanged()
{
    bool on = sensorDevice->on();
    if(on != isSensorAttached) {
        if(on) {
            addChildOnce(scene);
        } else {
            removeChild(scene);
        }
        isSensorAttached = on;
    }
}


SceneDevice* createSceneCollisionSensor(Device* device)
{
    return new SceneCollisionSensor(device);
}


CollisionSensor::CollisionSensor()
{
    on_ = false;
    color_ << 1.0, 0.0, 0.0;
}


CollisionSensor::CollisionSensor(const CollisionSensor& org, bool copyStateOnly)
  : ForceSensor(org, copyStateOnly)
{
    copyCollisionSensorStateFrom(org);
}


const char* CollisionSensor::typeName() const
{
    return "CollisionSensor";
}


void CollisionSensor::copyStateFrom(const DeviceState& other)
{
    if(typeid(other) != typeid(CollisionSensor)) {
        throw std::invalid_argument("Type mismatch in the Device::copyStateFrom function");
    }
    copyStateFrom(static_cast<const CollisionSensor&>(other));
}


void CollisionSensor::copyStateFrom(const CollisionSensor& other)
{
    ForceSensor::copyStateFrom(other);
    copyCollisionSensorStateFrom(other);
}


void CollisionSensor::copyCollisionSensorStateFrom(const CollisionSensor& other)
{
    on_ = other.on_;
    color_ = other.color_;
}


Referenced* CollisionSensor::doClone(CloneMap*) const
{
    return new CollisionSensor(*this, false);
}


DeviceState* CollisionSensor::cloneState() const
{
    return new CollisionSensor(*this, true);
}


void CollisionSensor::forEachActualType(std::function<bool(const std::type_info& type)> func)
{
    if(!func(typeid(CollisionSensor))) {
        ForceSensor::forEachActualType(func);
    }
}



void CollisionSensor::clearState()
{
    ForceSensor::clearState();
}


int CollisionSensor::stateSize() const
{
    return 4 + ForceSensor::stateSize();
}


const double* CollisionSensor::readState(const double* buf)
{
    int i = ForceSensor::stateSize();
    ForceSensor::readState(buf);
    on_ = buf[i++];
    color_[0] = buf[i++];
    color_[1] = buf[i++];
    color_[2] = buf[i++];
    return buf + i;
}


double* CollisionSensor::writeState(double* out_buf) const
{
    int i = ForceSensor::stateSize();
    ForceSensor::writeState(out_buf);
    out_buf[i++] = on_ ? 1.0 : 0.0;
    out_buf[i++] = color_[0];
    out_buf[i++] = color_[1];
    out_buf[i++] = color_[2];
    return out_buf + i;
}


bool CollisionSensor::on() const
{
    return on_;
}


void CollisionSensor::on(bool on)
{
    on_ = on;
}


bool CollisionSensor::readSpecifications(const Mapping* info)
{
    if(!ForceSensor::readSpecifications(info)) {
        return false;
    }

    info->read("on", on_);
    read(info, "color", color_);

    return true;
}


bool CollisionSensor::writeSpecifications(Mapping* info) const
{
    if(!ForceSensor::writeSpecifications(info)) {
        return false;
    }

    info->write("on", on_);
    write(info, "color", color_);

    return true;
}


namespace {

bool readCollisionSensor(StdBodyLoader* loader, const Mapping* info)
{
    CollisionSensorPtr sensor = new CollisionSensor;
    if(!sensor->readSpecifications(info)) {
        sensor.reset();
    }

    bool result = false;
    if(sensor) {
        result = loader->readDevice(sensor, info);
    }
    return result;
}


struct CollisionSensorRegistration
{
    CollisionSensorRegistration() {
        StdBodyLoader::registerNodeType("CollisionSensor", readCollisionSensor);
        StdBodyWriter::registerDeviceWriter<CollisionSensor>(
                    "CollisionSensor",
                    [](StdBodyWriter* /* writer */, Mapping* info, const CollisionSensor* sensor) {
            return sensor->writeSpecifications(info);
        });
        SceneDevice::registerSceneDeviceFactory<CollisionSensor>(createSceneCollisionSensor);
    }
} registration;

}
