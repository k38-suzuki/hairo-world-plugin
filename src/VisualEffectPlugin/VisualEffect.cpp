#include "VisualEffect.h"

using namespace cnoid;

namespace cnoid {

class VisualEffectImpl
{
public:
    VisualEffectImpl(VisualEffect* self);

    VisualEffect* self;
    double hue;
    double saturation;
    double value;
    double red;
    double green;
    double blue;
    double coefB;
    double coefD;
    double stdDev;
    double salt;
    double pepper;
    bool flip;
};

}


VisualEffect::VisualEffect()
{
    impl = new VisualEffectImpl(this);
}


VisualEffectImpl::VisualEffectImpl(VisualEffect* self)
    : self(self)
{
    hue = 0.0;
    saturation = 0.0;
    value = 0.0;
    red = 0.0;
    green = 0.0;
    blue = 0.0;
    coefB = 0.0;
    coefD = 1.0;
    stdDev = 0.0;
    salt = 0.0;
    pepper = 0.0;
    flip = false;
}


VisualEffect::~VisualEffect()
{
    delete impl;
}


void VisualEffect::setHue(const double hue)
{
    impl->hue = hue;
}


double VisualEffect::hue() const
{
    return impl->hue;
}


void VisualEffect::setSaturation(const double saturation)
{
    impl->saturation = saturation;
}


double VisualEffect::saturation() const
{
    return impl->saturation;
}


void VisualEffect::setValue(const double value)
{
    impl->value = value;
}


double VisualEffect::value() const
{
    return impl->value;
}


void VisualEffect::setRed(const double red)
{
    impl->red = red;
}


double VisualEffect::red() const
{
    return impl->red;
}


void VisualEffect::setGreen(const double green)
{
    impl->green = green;
}


double VisualEffect::green() const
{
    return impl->green;
}


void VisualEffect::setBlue(const double blue)
{
    impl->blue = blue;
}


double VisualEffect::blue() const
{
    return impl->blue;
}


void VisualEffect::setCoefB(const double coefB)
{
    impl->coefB = coefB;
}


double VisualEffect::coefB() const
{
    return impl->coefB;
}


void VisualEffect::setCoefD(const double coefD)
{
    impl->coefD = coefD;
}


double VisualEffect::coefD() const
{
    return impl->coefD;
}


void VisualEffect::setStdDev(const double stdDev)
{
    impl->stdDev = stdDev;
}


double VisualEffect::stdDev() const
{
    return impl->stdDev;
}


void VisualEffect::setSalt(const double salt)
{
    impl->salt = salt;
}


double VisualEffect::salt() const
{
    return impl->salt;
}


void VisualEffect::setPepper(const double pepper)
{
    impl->pepper = pepper;
}


double VisualEffect::pepper() const
{
    return impl->pepper;
}


void VisualEffect::setFlip(const bool flip)
{
    impl->flip = flip;
}


bool VisualEffect::flip() const
{
    return impl->flip;
}
