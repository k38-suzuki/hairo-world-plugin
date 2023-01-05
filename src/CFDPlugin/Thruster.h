/**
   \file
   \author Kenta Suzuki
*/

#ifndef CNOID_CFD_PLUGIN_THRUSTER_H
#define CNOID_CFD_PLUGIN_THRUSTER_H

#include <cnoid/Device>
#include <cnoid/ValueTree>
#include "exportdecl.h"

namespace cnoid {

class CNOID_EXPORT Thruster : public Device
{
public:
    Thruster();
    Thruster(const Thruster& org, bool copyStateOnly = false);
    virtual const char* typeName() const override;
    void copyStateFrom(const Thruster& other);
    virtual void copyStateFrom(const DeviceState& other) override;
    virtual DeviceState* cloneState() const override;
    virtual void forEachActualType(std::function<bool(const std::type_info& type)> func) override;
    virtual void clearState() override;
    virtual bool on() const override;
    virtual void on(bool on) override;
    virtual int stateSize() const override;
    virtual const double* readState(const double* buf) override;
    virtual double* writeState(double* out_buf) const override;

    bool readSpecifications(const Mapping* info);
    bool writeSpecifications(Mapping* info) const;

    double& force() { return force_; }
    double& torque() { return torque_; }
    void setDirection(const Vector3& direction) { direction_ = direction; }
    Vector3 direction() const { return direction_; }
    void setForceOffset(const double& forceOffset) { forceOffset_ = forceOffset; }
    double forceOffset() const { return forceOffset_; }
    void setTorqueOffset(const double& torqueOffset) { torqueOffset_ = torqueOffset; }
    double torqueOffset() const { return torqueOffset_; }
    void setSymbol(const bool& symbol) { symbol_ = symbol; }
    bool symbol() const { return symbol_; }

protected:
    virtual Referenced* doClone(CloneMap* cloneMap) const override;

private:
    bool on_;
    double force_;
    double torque_;
    Vector3 direction_;
    double forceOffset_;
    double torqueOffset_;
    bool symbol_;

    void copyThrusterStateFrom(const Thruster& other);
};

typedef ref_ptr<Thruster> ThrusterPtr;

}

#endif // CNOID_CFD_PLUGIN_THRUSTER_H