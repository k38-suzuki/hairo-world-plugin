#include "VisualEffect.h"

using namespace cnoid;

VisualEffect::VisualEffect()
{
    hue_ = 0.0;
    saturation_ = 0.0;
    value_ = 0.0;
    red_ = 0.0;
    green_ = 0.0;
    blue_ = 0.0;
    coefB_ = 0.0;
    coefD_ = 1.0;
    stdDev_ = 0.0;
    salt_ = 0.0;
    pepper_ = 0.0;
    flip_ = false;
    filter_ = 0;
}


VisualEffect::~VisualEffect()
{

}
