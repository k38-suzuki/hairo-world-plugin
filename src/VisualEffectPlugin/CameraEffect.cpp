/**
   @author Kenta Suzuki
*/

#include "CameraEffect.h"

using namespace std;
using namespace cnoid;

CameraEffect::CameraEffect()
{
    hsv_ << 0.0, 0.0, 0.0;
    rgb_ << 0.0, 0.0, 0.0;
    coef_b_ = 0.0;
    coef_d_ = 0.0;
    std_dev_ = 0.0;
    salt_ = 0.0;
    pepper_ = 0.0;
    flip_ = false;
    filterType_ = NO_FILTER;
}


CameraEffect::~CameraEffect()
{

}
