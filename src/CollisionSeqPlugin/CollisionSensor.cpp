/**
   \file
   \author Kenta Suzuki
*/

#include "CollisionSensor.h"
#include <cnoid/EigenArchive>
#include <cnoid/StdBodyFileUtil>
#include <cnoid/ValueTree>

using namespace cnoid;
using namespace std;


CollisionSensor::CollisionSensor()
{
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
    if(typeid(other) != typeid(CollisionSensor)){
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
    if(!func(typeid(CollisionSensor))){
        ForceSensor::forEachActualType(func);
    }
}



void CollisionSensor::clearState()
{
    ForceSensor::clearState();
}


int CollisionSensor::stateSize() const
{
    return 3;
}


const double* CollisionSensor::readState(const double* buf)
{
    int i = 0;
    color_[i++] = buf[0];
    color_[i++] = buf[1];
    color_[i++] = buf[2];
    return buf + i;
}


double* CollisionSensor::writeState(double* out_buf) const
{
    int i = 0;
    out_buf[i++] = color_[0];
    out_buf[i++] = color_[1];
    out_buf[i++] = color_[2];
    return out_buf + i;
}


bool CollisionSensor::readSpecifications(const Mapping* info)
{
    if(!ForceSensor::readSpecifications(info)){
        return false;
    }

    read(info, "color", color_);

    return true;
}


bool CollisionSensor::writeSpecifications(Mapping* info) const
{
    if(!ForceSensor::writeSpecifications(info)){
        return false;
    }

    write(info, "color", color_);

    return true;
}


namespace {

StdBodyFileDeviceTypeRegistration<CollisionSensor>
registerHolderDevice(
    "CollisionSensor",
     [](StdBodyLoader* loader, const Mapping* info){
         CollisionSensorPtr sensor = new CollisionSensor;
         if(sensor->readSpecifications(info)){
            return loader->readDevice(sensor, info);
        }
        return false;
    },
    [](StdBodyWriter* /* writer */, Mapping* info, const CollisionSensor* sensor)
    {
        return sensor->writeSpecifications(info);
    });
}
