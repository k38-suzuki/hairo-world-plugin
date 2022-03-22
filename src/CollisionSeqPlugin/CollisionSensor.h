/**
   \file
   \author Kenta Suzuki
*/

#ifndef CNOID_COLLISIONSEQPLUGIN_COLLISIONSENSOR_H
#define CNOID_COLLISIONSEQPLUGIN_COLLISIONSENSOR_H

#include <cnoid/ForceSensor>
#include "exportdecl.h"

namespace cnoid {

class CNOID_EXPORT CollisionSensor : public ForceSensor
{
public:
    CollisionSensor();
    CollisionSensor(const CollisionSensor& org, bool copyStateOnly = false);

    virtual const char* typeName() const override;
    void copyStateFrom(const CollisionSensor& other);
    virtual void copyStateFrom(const DeviceState& other) override;
    virtual DeviceState* cloneState() const override;
    virtual void forEachActualType(std::function<bool(const std::type_info& type)> func) override;
    virtual void clearState() override;

    Vector3 color() const { return color_; }
    void setColor(const Vector3 color) { color_ = color; }

    virtual int stateSize() const override;
    virtual const double* readState(const double* buf) override;
    virtual double* writeState(double* out_buf) const override;

    virtual bool on() const override;
    virtual void on(bool on) override;

    bool readSpecifications(const Mapping* info);
    bool writeSpecifications(Mapping* info) const;

protected:
    virtual Referenced* doClone(CloneMap* cloneMap) const override;

private:
    bool on_;
    Vector3 color_;

    void copyCollisionSensorStateFrom(const CollisionSensor& other);
};

typedef ref_ptr<CollisionSensor> CollisionSensorPtr;

}

#endif // CNOID_COLLISIONSEQPLUGIN_COLLISIONSENSOR_H
