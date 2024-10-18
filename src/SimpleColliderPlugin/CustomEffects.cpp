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
    salt_amount_ = 0.0;
    salt_chance_ = 0.0;
    pepper_amount_ = 0.0;
    pepper_chance_ = 0.0;
    mosaic_chance_ = 0.0;
    kernel_ = 16;
}


VFXEffects::VFXEffects(const VFXEffects& org)
{
    hsv_ = org.hsv_;
    rgb_ = org.rgb_;
    coef_b_ = org.coef_b_;
    coef_d_ = org.coef_d_;
    std_dev_ = org.std_dev_;
    salt_amount_ = org.salt_amount_;
    salt_chance_ = org.salt_chance_;
    pepper_amount_ = org.pepper_amount_;
    pepper_chance_ = org.pepper_chance_;
    mosaic_chance_ = org.mosaic_chance_;
    kernel_ = org.kernel_;
}
