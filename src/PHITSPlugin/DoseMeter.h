/**
   @author Kenta Suzuki
*/

#ifndef CNOID_PHITS_PLUGIN_DOSE_METER_H
#define CNOID_PHITS_PLUGIN_DOSE_METER_H

#include <cnoid/Device>
#include <cnoid/EigenTypes>
#include <cnoid/ValueTree>
#include "exportdecl.h"

namespace cnoid {

class CNOID_EXPORT DoseMeter : public Device
{
public:
    DoseMeter();
    DoseMeter(const DoseMeter& org, bool copyStateOnly = false);
    virtual const char* typeName() const override;
    void copyStateFrom(const DoseMeter& other);
    virtual void copyStateFrom(const DeviceState& other) override;
    virtual DeviceState* cloneState() const override;
    virtual void forEachActualType(std::function<bool(const std::type_info& type)> func) override;
    virtual void clearState() override;
    virtual int stateSize() const override;
    virtual const double* readState(const double* buf) override;
    virtual double* writeState(double* out_buf) const override;

    void setIntegralDose(const double integralDose) { integralDose_[1] = integralDose; }
    double integralDose() const { return integralDose_[1]; }
    void setDoseRate(const double doseRate) { doseRate_ = doseRate; }
    double doseRate() const { return doseRate_; }
    void setShield(bool on);
    bool isShield()  const { return isShield_; }
    void setMaterial(const std::string material) { material_ = material; }
    std::string material() const { return material_; }
    void setShieldThickness(const double thickness) { thickness_ = thickness; }
    double thickness() const { return thickness_; }
    void setColor(const Vector3& color) { color_ = color; }
    Vector3 color() const { return color_; }

    virtual bool on() const override;
    virtual void on(bool on) override;

    bool readSpecifications(const Mapping* info);
    bool writeSpecifications(Mapping* info) const;

protected:
    virtual Referenced* doClone(CloneMap* cloneMap) const override;

private:
    bool on_;
    Vector3 color_;
    Vector2 integralDose_;
    double doseRate_;
    bool hasShield_;
    bool isShield_;
    std::string material_;
    double thickness_;
};

typedef ref_ptr<DoseMeter> DoseMeterPtr;

}

#endif // CNOID_PHITS_PLUGIN_DOSE_METER_H
