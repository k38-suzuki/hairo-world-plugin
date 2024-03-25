/**
   @author Kenta Suzuki
*/

#include "CameraEffects.h"

using namespace std;
using namespace cnoid;

CameraEffects::CameraEffects()
{
    hsv_ << 0.0, 0.0, 0.0;
    rgb_ << 0.0, 0.0, 0.0;
    coef_b_ = 0.0;
    coef_d_ = 0.0;
    std_dev_ = 0.0;
    salt_ = 0.0;
    pepper_ = 0.0;
    flipped_ = false;
    filterType_ = NO_FILTER;
}


CameraEffects::CameraEffects(const CameraEffects& org)
{
    hsv_ = org.hsv_;
    rgb_ = org.rgb_;
    coef_b_ = org.coef_b_;
    coef_d_ = org.coef_d_;
    std_dev_ = org.std_dev_;
    salt_ = org.salt_;
    pepper_ = org.pepper_;
    flipped_ = org.flipped_;
    filterType_ = org.filterType_;
}


CameraEffects::~CameraEffects()
{

}
