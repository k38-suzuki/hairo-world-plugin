/**
   @author Kenta Suzuki
*/

#ifndef CNOID_VFX_PLUGIN_IMAGE_GENERATOR_H
#define CNOID_VFX_PLUGIN_IMAGE_GENERATOR_H

#include <cnoid/Image>
#include <QImage>

namespace cnoid {

class CNOID_EXPORT ImageGenerator
{
public:
    ImageGenerator();
    virtual ~ImageGenerator();

    void barrelDistortion(Image& image, const double& m_coefb, const double& m_coefd);
    void gaussianNoise(Image& image, const double& m_std_dev);
    void hsv(Image& image, const double& m_hue, const double& m_saturation, const double& m_value);
    void rgb(Image& image, const double& m_red, const double& m_green, const double& m_blue);
    void saltPepperNoise(Image& image, const double& m_salt, const double& m_pepper);
    void filteredImage(Image& image, const double& m_scalex, const double& m_scaley);
    void flippedImage(Image& image);
    void gaussianFilter(Image& image, const int& matrix);
    void medianFilter(Image& image, const int& matrix);
    void sobelFilter(Image& image);
    void prewittFilter(Image& image);

private:
    class Impl;
    Impl* impl;
};

CNOID_EXPORT Image toCnoidImage(const QImage& image);
CNOID_EXPORT QImage toQImage(const Image& image);

}

#endif
