/**
   @author Kenta Suzuki
*/

#include "CustomEffect.h"
#include <cnoid/EigenArchive>

using namespace cnoid;

CFDEffect::CFDEffect()
{
    density_ = 0.0;
    viscosity_ = 0.0;
    steadyFlow_ << 0.0, 0.0, 0.0;
    unsteadyFlow_ << 0.0, 0.0, 0.0;
}


CFDEffect::CFDEffect(const CFDEffect& org)
{
    density_ = org.density_;
    viscosity_ = org.viscosity_;
    steadyFlow_ = org.steadyFlow_;
    unsteadyFlow_ = org.unsteadyFlow_;
}


TCEffect::TCEffect()
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


TCEffect::TCEffect(const TCEffect& org)
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


VisualEffect::VisualEffect()
{
    hsv_ << 0.0, 0.0, 0.0;
    rgb_ << 0.0, 0.0, 0.0;
    coef_b_ = 0.0;
    coef_d_ = 1.0;
    std_dev_ = 0.0;
    salt_amount_ = 0.0;
    salt_chance_ = 0.0;
    pepper_amount_ = 0.0;
    pepper_chance_ = 0.0;
    mosaic_chance_ = 0.0;
    kernel_ = 8;
}


VisualEffect::VisualEffect(const VisualEffect& org)
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


bool VisualEffect::readCameraInfo(const Mapping* info)
{
    read(info, "hsv", hsv_);
    if(hsv_[0] < 0.0 || hsv_[0] > 1.0
        || hsv_[1] < 0.0 || hsv_[1] > 1.0
        || hsv_[2] < 0.0 || hsv_[2] > 1.0) {
        return false;
    }

    read(info, "rgb", rgb_);
    if(rgb_[0] < 0.0 || rgb_[0] > 1.0
        || rgb_[1] < 0.0 || rgb_[1] > 1.0
        || rgb_[2] < 0.0 || rgb_[2] > 1.0) {
        return false;
    }

    coef_b_ = info->get({ "coef_b", "coefB" }, 0.0);
    if(coef_b_ < -1.0 || coef_b_ > 0.0) {
        return false;
    }

    coef_d_ = info->get({ "coef_d", "coefD" }, 1.0);
    if(coef_d_ < 1.0 || coef_d_ > 32.0) {
        return false;
    }

    std_dev_ = info->get({ "std_dev", "stdDev" }, 0.0);
    if(std_dev_ < 0.0 || std_dev_ > 1.0) {
        return false;
    }

    salt_amount_ = info->get({ "salt_amount", "saltAmount" }, 0.0);
    if(salt_amount_ < 0.0 || salt_amount_ > 1.0) {
        return false;
    }

    salt_chance_ = info->get({ "salt_chance", "saltChance" }, 0.0);
    if(salt_chance_ < 0.0 || salt_chance_ > 1.0) {
        return false;
    }

    pepper_amount_ = info->get({ "pepper_amount", "pepperAmount" }, 0.0);
    if(pepper_amount_ < 0.0 || pepper_amount_ > 1.0) {
        return false;
    }

    pepper_chance_ = info->get({ "pepper_chance", "pepperChance" }, 0.0);
    if(pepper_chance_ < 0.0 || pepper_chance_ > 1.0) {
        return false;
    }

    mosaic_chance_ = info->get({ "mosaic_chance", "mosaicChance" }, 0.0);
    if(mosaic_chance_ < 0.0 || mosaic_chance_ > 1.0) {
        return false;
    }

    kernel_ = info->get("kernel", 8);
    if(kernel_ < 8 || kernel_ > 64) {
        return false;
    }

    return true;
}