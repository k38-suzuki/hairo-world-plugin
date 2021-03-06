/**
   \file
   \author Kenta Suzuki
*/

#ifndef CNOID_FLUID_DYNAMICS_PLUGIN_ROTOR_H
#define CNOID_FLUID_DYNAMICS_PLUGIN_ROTOR_H

#include <cnoid/Device>
#include "exportdecl.h"

namespace cnoid {

class CNOID_EXPORT Rotor : public Device
{
public:
    Rotor();
    Rotor(const Rotor& org, bool copyStateOnly = false);
    virtual const char* typeName() const override;
    void copyStateFrom(const Rotor& other);
    virtual void copyStateFrom(const DeviceState& other) override;
    virtual DeviceState* cloneState() const override;
    virtual void forEachActualType(std::function<bool(const std::type_info& type)> func) override;
    virtual void clearState() override;
    virtual bool on() const override;
    virtual void on(bool on) override;
    virtual int stateSize() const override;
    virtual const double* readState(const double* buf) override;
    virtual double* writeState(double* out_buf) const override;

    double& force() { return force_; }
    double& torque() { return torque_; }
    void setForceOffset(const double& forceOffset) { forceOffset_ = forceOffset; }
    double forceOffset() const { return forceOffset_; }
    void setTorqueOffset(const double& torqueOffset) { torqueOffset_ = torqueOffset; }
    double torqueOffset() const { return torqueOffset_; }

    void setK(const double& k) { k_ = k; }
    double k() const { return k_; }
    void setKv(const double& kv) { kv_ = kv; }
    double kv() const { return kv_; }
    void setDiameter(const double& diameter) { diameter_ = diameter; }
    double diameter() const { return diameter_; }
    void setPitch(const double& pitch) { pitch_ = pitch; }
    double pitch() const { return pitch_; }
    double& voltage() { return voltage_; }
    void setReverse(const bool& reverse) { reverse_ = reverse; }
    bool reverse() const { return reverse_; }

protected:
    virtual Referenced* doClone(CloneMap* cloneMap) const override;

private:
    bool on_;
    double force_;
    double torque_;
    double forceOffset_;
    double torqueOffset_;
    double k_;
    double kv_;
    double diameter_;
    double pitch_;
    double voltage_;
    bool reverse_;
};

typedef ref_ptr<Rotor> RotorPtr;

}

#endif // CNOID_FLUID_DYNAMICS_PLUGIN_ROTOR_H
