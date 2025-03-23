/**
   @author Kenta Suzuki
*/

#ifndef CNOID_VFX_PLUGIN_VISUAL_FILTER_H
#define CNOID_VFX_PLUGIN_VISUAL_FILTER_H

#include <cnoid/Image>
#include <QImage>
#include <random>
#include <memory>
#include "exportdecl.h"

namespace cnoid {

class CNOID_EXPORT VisualFilter
{
public:
    VisualFilter();

    void initialize(int width, int height);

    void red(Image* image);
    void green(Image* image);
    void blue(Image* image);
    void white(Image* image);
    void black(Image* image);

    void salt(Image* image, const double& salt_amount);
    void random_salt(Image* image, const double& salt_amount, const double& salt_chance);
    void pepper(Image* image, const double& pepper_amount);
    void random_pepper(Image* image, const double& pepper_amount, const double& pepper_chance);
    void salt_pepper(Image* image, const double& salt_amount, const double& pepper_amount);
    void rgb(Image* image, const double& red, const double& green, const double& blue);
    void hsv(Image* image, const double& hue, const double& saturation, const double& value);
    void gaussian_noise(Image* image, const double& std_dev);
    void barrel_distortion(Image* image, const double& coef_b, const double& coef_d);
    void mosaic(Image* image, int kernel = 16);
    void random_mosaic(Image* image, const double& rate, int kernel = 16);

private:
    int width_;
    int height_;
    std::random_device seed_gen_;
    std::default_random_engine engine_;
    std::normal_distribution<> dist_;
};

void toCnoidImage(Image* image, QImage q_image);
QImage toQImage(Image* image);

}

#endif // CNOID_VFX_PLUGIN_VISUAL_FILTER_H
