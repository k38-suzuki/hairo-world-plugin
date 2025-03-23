/**
   @author Kenta Suzuki
*/

#ifndef CNOID_SIMPLECOLLIDER_PLUGIN_CUSTOM_EFFECT_H
#define CNOID_SIMPLECOLLIDER_PLUGIN_CUSTOM_EFFECT_H

#include <cnoid/EigenUtil>
#include <cnoid/ValueTree>
#include "exportdecl.h"

namespace cnoid {

class CNOID_EXPORT CFDEffect
{
public:
    CFDEffect();
    CFDEffect(const CFDEffect& org);

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

class CNOID_EXPORT TCEffect
{
public:
    TCEffect();
    TCEffect(const TCEffect& org);

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

class CNOID_EXPORT VisualEffect
{
public:
    VisualEffect();
    VisualEffect(const VisualEffect& org);

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
    void setSaltAmount(const double& salt_amount) { salt_amount_ = salt_amount; }
    double saltAmount() const { return salt_amount_; }
    void setSaltChance(const double& salt_chance) { salt_chance_ = salt_chance; }
    double saltChance() const { return salt_chance_; }
    void setPepperAmount(const double& pepper_amount) { pepper_amount_ = pepper_amount; }
    double pepperAmount() const { return pepper_amount_; }
    void setPepperChance(const double& pepper_chance) { pepper_chance_ = pepper_chance; }
    double pepperChance() const { return pepper_chance_; }
    void setMosaicChance(const double& mosaic_chance) { mosaic_chance_ = mosaic_chance; }
    double mosaicChance() const { return mosaic_chance_; }
    void setKernel(const int& kernel) { kernel_ = kernel; }
    int kernel() const { return kernel_; }

    bool readCameraInfo(const Mapping* info);

private:
    Vector3 hsv_;
    Vector3 rgb_;
    double coef_b_;
    double coef_d_;
    double std_dev_;
    double salt_amount_;
    double salt_chance_;
    double pepper_amount_;
    double pepper_chance_;
    double mosaic_chance_;
    int kernel_;
};

}

#endif // CNOID_SIMPLECOLLIDER_PLUGIN_CUSTOM_EFFECT_H