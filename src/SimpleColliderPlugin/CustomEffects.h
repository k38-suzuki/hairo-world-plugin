/**
   @author Kenta Suzuki
*/

#ifndef CNOID_SIMPLECOLLIDER_PLUGIN_CUSTOM_EFFECTS_H
#define CNOID_SIMPLECOLLIDER_PLUGIN_CUSTOM_EFFECTS_H

#include <cnoid/EigenUtil>
#include "exportdecl.h"

namespace cnoid {

class CNOID_EXPORT CFDEffects
{
public:
    CFDEffects();
    CFDEffects(const CFDEffects& org);

    void setDensity(const double& density) { density_ = density; }
    double density() const { return density_; }
    void setViscosity(const double& viscosity) { viscosity_ = viscosity; }
    double viscosity() const { return viscosity_; }
    void setSteadyFlow(const Vector3& steadyFlow) { steadyFlow_ = steadyFlow; }
    Vector3 steadyFlow() const { return steadyFlow_; }
    void setUnsteadyFlow(const Vector3& unsteadyFlow) { unsteadyFlow_ = unsteadyFlow; }
    Vector3 unsteadyFlow() const { return unsteadyFlow_; }

private:
    double density_;
    double viscosity_;
    Vector3 steadyFlow_;
    Vector3 unsteadyFlow_;
};

class CNOID_EXPORT TCEffects
{
public:
    TCEffects();
    TCEffects(const TCEffects& org);

    void setInboundDelay(const double& inboundDelay) { inboundDelay_ = inboundDelay; }
    double inboundDelay() const { return inboundDelay_; }
    void setInboundRate(const double& inboundRate) { inboundRate_ = inboundRate; }
    double inboundRate() const { return inboundRate_; }
    void setInboundLoss(const double& inboundLoss) { inboundLoss_ = inboundLoss; }
    double inboundLoss() const { return inboundLoss_; }
    void setOutboundDelay(const double& outboundDelay) { outboundDelay_ = outboundDelay; }
    double outboundDelay() const { return outboundDelay_; }
    void setOutboundRate(const double& outboundRate) { outboundRate_ = outboundRate; }
    double outboundRate() const { return outboundRate_; }
    void setOutboundLoss(const double& outboundLoss) { outboundLoss_ = outboundLoss; }
    double outboundLoss() const { return outboundLoss_; }
    void setSource(const std::string& source) { source_ = source; }
    std::string source() const { return source_; }
    void setDestination(const std::string& destination){ destination_ = destination; }
    std::string destination() const { return destination_; }

private:
    double inboundDelay_;
    double inboundRate_;
    double inboundLoss_;
    double outboundDelay_;
    double outboundRate_;
    double outboundLoss_;
    std::string source_;
    std::string destination_;
};

class CNOID_EXPORT VFXEffects
{
public:
    VFXEffects();
    VFXEffects(const VFXEffects& org);

    void setHsv(const Vector3& hsv) { hsv_ = hsv; }
    Vector3 hsv() const { return hsv_; }
    void setRgb(const Vector3& rgb) { rgb_ = rgb; }
    Vector3 rgb() const { return rgb_; }
    void setCoefB(const double coef_b) { coef_b_ = coef_b; }
    double coefB() const { return coef_b_; }
    void setCoefD(const double& coef_d) { coef_d_ = coef_d; }
    double coefD() const { return coef_d_; }
    void setStdDev(const double& std_dev) { std_dev_ = std_dev; }
    double stdDev() const { return std_dev_; }
    void setSalt(const double& salt) { salt_ = salt; }
    double salt() const { return salt_; }
    void setPepper(const double& pepper) { pepper_ = pepper; }
    double pepper() const { return pepper_; }

private:
    Vector3 hsv_;
    Vector3 rgb_;
    double coef_b_;
    double coef_d_;
    double std_dev_;
    double salt_;
    double pepper_;
};

}

#endif // CNOID_SIMPLECOLLIDER_PLUGIN_CUSTOM_EFFECTS_H