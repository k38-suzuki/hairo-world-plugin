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

#endif // CNOID_VFX_PLUGIN_IMAGE_GENERATOR_H
