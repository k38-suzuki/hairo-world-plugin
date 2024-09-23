/**
   @author Kenta Suzuki
*/

#ifndef CNOID_CFD_PLUGIN_ROTOR_H
#define CNOID_CFD_PLUGIN_ROTOR_H

#include "Thruster.h"
#include "exportdecl.h"

namespace cnoid {

class Mapping;

class CNOID_EXPORT Rotor : public Thruster
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
    virtual int stateSize() const override;
    virtual const double* readState(const double* buf) override;
    virtual double* writeState(double* out_buf) const override;

    bool readSpecifications(const Mapping* info);
    bool writeSpecifications(Mapping* info) const;

    void setK(const double& k) { k_ = k; }
    double k() const { return k_; }
    void setKv(const double& kv) { kv_ = kv; }
    double kv() const { return kv_; }
    void setDiameter(const double& diameter) { diameter_ = diameter; }
    double diameter() const { return diameter_; }
    void setPitch(const double& pitch) { pitch_ = pitch; }
    double pitch() const { return pitch_; }
    double& voltage() { return voltage_; }
    void setReverse(bool reverse) { reverse_ = reverse; }
    bool reverse() const { return reverse_; }

protected:
    virtual Referenced* doClone(CloneMap* cloneMap) const override;

private:
    double k_;
    double kv_;
    double diameter_;
    double pitch_;
    double voltage_;
    bool reverse_;

    struct Spec {

    };
    std::unique_ptr<Spec> spec;

    void copyRotorStateFrom(const Rotor& other);
};

typedef ref_ptr<Rotor> RotorPtr;

}

#endif // CNOID_CFD_PLUGIN_ROTOR_H
