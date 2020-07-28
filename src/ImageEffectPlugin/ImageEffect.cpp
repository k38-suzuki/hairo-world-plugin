#include "ImageEffect.h"

using namespace cnoid;

namespace cnoid {

class ImageEffectImpl
{
public:
    ImageEffectImpl(ImageEffect* self);

    ImageEffect* self;
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
};

}


ImageEffect::ImageEffect()
{
    impl = new ImageEffectImpl(this);
}


ImageEffectImpl::ImageEffectImpl(ImageEffect* self)
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
}


ImageEffect::~ImageEffect()
{
    delete impl;
}


void ImageEffect::setHue(const double hue)
{
    impl->hue = hue;
}


double ImageEffect::hue() const
{
    return impl->hue;
}


void ImageEffect::setSaturation(const double saturation)
{
    impl->saturation = saturation;
}


double ImageEffect::saturation() const
{
    return impl->saturation;
}


void ImageEffect::setValue(const double value)
{
    impl->value = value;
}


double ImageEffect::value() const
{
    return impl->value;
}


void ImageEffect::setRed(const double red)
{
    impl->red = red;
}


double ImageEffect::red() const
{
    return impl->red;
}


void ImageEffect::setGreen(const double green)
{
    impl->green = green;
}


double ImageEffect::green() const
{
    return impl->green;
}


void ImageEffect::setBlue(const double blue)
{
    impl->blue = blue;
}


double ImageEffect::blue() const
{
    return impl->blue;
}


void ImageEffect::setCoefB(const double coefB)
{
    impl->coefB = coefB;
}


double ImageEffect::coefB() const
{
    return impl->coefB;
}


void ImageEffect::setCoefD(const double coefD)
{
    impl->coefD = coefD;
}


double ImageEffect::coefD() const
{
    return impl->coefD;
}


void ImageEffect::setStdDev(const double stdDev)
{
    impl->stdDev = stdDev;
}


double ImageEffect::stdDev() const
{
    return impl->stdDev;
}


void ImageEffect::setSalt(const double salt)
{
    impl->salt = salt;
}


double ImageEffect::salt() const
{
    return impl->salt;
}


void ImageEffect::setPepper(const double pepper)
{
    impl->pepper = pepper;
}


double ImageEffect::pepper() const
{
    return impl->pepper;
}
