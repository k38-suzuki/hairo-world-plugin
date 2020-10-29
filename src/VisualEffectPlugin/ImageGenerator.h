/**
   \file
   \author Kenta Suzuki
*/

#ifndef CNOID_VISUAL_EFFECT_PLUGIN_IMAGE_GENERATOR_H
#define CNOID_VISUAL_EFFECT_PLUGIN_IMAGE_GENERATOR_H

#include <cnoid/Image>

namespace cnoid {

class ImageGeneratorImpl;

class ImageGenerator
{
public:
    ImageGenerator();
    virtual ~ImageGenerator();

    Image barrelDistortion(const Image image, const double m_coefb, double m_coefd);
    Image gaussianNoise(const Image image, const double m_std_dev);
    Image hsv(const Image image, const double m_hue, const double m_saturation, const double m_value);
    Image rgb(const Image image, const double m_red, const double m_green, const double m_blue);
    Image saltPepperNoise(const Image image, const double m_salt, const double m_pepper);
    Image filteredImage(Image image, double m_scalex, double m_scaley);

private:
    ImageGeneratorImpl* impl;
    friend class ImageGeneratorImpl;
};

}

#endif // CNOID_VISUAL_EFFECT_PLUGIN_IMAGE_GENERATOR_H
