/**
   @author Kenta Suzuki
*/

#include "CustomEffects.h"

using namespace cnoid;

CFDEffects::CFDEffects()
{
    density_ = 0.0;
    viscosity_ = 0.0;
    steadyFlow_ << 0.0, 0.0, 0.0;
    unsteadyFlow_ << 0.0, 0.0, 0.0;
}


CFDEffects::CFDEffects(const CFDEffects& org)
{
    density_ = org.density_;
    viscosity_ = org.viscosity_;
    steadyFlow_ = org.steadyFlow_;
    unsteadyFlow_ = org.unsteadyFlow_;
}


TCEffects::TCEffects()
{
    inboundDelay_ = 0.0;
    inboundRate_ = 0.0;
    inboundLoss_ = 0.0;
    outboundDelay_ = 0.0;
    outboundRate_ = 0.0;
    outboundLoss_ = 0.0;
    source_ = "0.0.0.0/0";
    destination_ = "0.0.0.0/0";
}


TCEffects::TCEffects(const TCEffects& org)
{
    inboundDelay_ = org.inboundDelay_;
    inboundRate_ = org.inboundRate_;
    inboundLoss_ = org.inboundLoss_;
    outboundDelay_ = org.outboundDelay_;
    outboundRate_ = org.outboundRate_;
    outboundLoss_ = org.outboundLoss_;
    source_ = org.source_;
    destination_ = org.destination_;
}


VFXEffects::VFXEffects()
{
    hsv_ << 0.0, 0.0, 0.0;
    rgb_ << 0.0, 0.0, 0.0;
    coef_b_ = 0.0;
    coef_d_ = 0.0;
    std_dev_ = 0.0;
    salt_ = 0.0;
    salt_rate_ = 1.0;
    pepper_ = 0.0;
    pepper_rate_ = 1.0;
    mosaic_rate_ = 1.0;
    kernel_ = 16;
}


VFXEffects::VFXEffects(const VFXEffects& org)
{
    hsv_ = org.hsv_;
    rgb_ = org.rgb_;
    coef_b_ = org.coef_b_;
    coef_d_ = org.coef_d_;
    std_dev_ = org.std_dev_;
    salt_ = org.salt_;
    salt_rate_ = org.salt_rate_;
    pepper_ = org.pepper_;
    pepper_rate_ = org.pepper_rate_;
    mosaic_rate_ = org.mosaic_rate_;
    kernel_ = org.kernel_;
}
