/**
   @author Kenta Suzuki
*/

#ifndef CNOID_VISUAL_EFFECT_PLUGIN_CAMERA_EFFECT_H
#define CNOID_VISUAL_EFFECT_PLUGIN_CAMERA_EFFECT_H

#include <cnoid/EigenTypes>
#include "exportdecl.h"

namespace cnoid {

class CNOID_EXPORT CameraEffect
{
public:
    CameraEffect();
    ~CameraEffect();

    enum FilterType { NO_FILTER, GAUSSIAN_3X3, GAUSSIAN_5X5, SOBEL, PREWITT };

    void setHsv(const Vector3& hsv) { hsv_ = hsv; }
    Vector3 hsv() const { return hsv_; }
    void setRgb(const Vector3& rgb) { rgb_ = rgb; }
    Vector3 rgb() const { return rgb_; }
    void setCoefB(const double coef_b) { coef_b_ = coef_b; }
    double coefB() const { return coef_b_; }
    void setCoefD(const double& coef_d) { coef_d_ = coef_d; }
    double coefD() const { return coef_d_; }
    void setStdDev(const double& std_dev) { std_dev_ = std_dev; }
    double stdDev() const { return std_dev_; }
    void setSalt(const double& salt) { salt_ = salt; }
    double salt() const { return salt_; }
    void setPepper(const double& pepper) { pepper_ = pepper; }
    double pepper() const { return pepper_; }
    void setFlip(const bool& flip) { flip_ = flip; }
    bool flip() const { return flip_; }
    void setFilterType(const FilterType& filterType) { filterType_ = filterType; }
    FilterType filterType() const { return filterType_; }

private:
    Vector3 hsv_;
    Vector3 rgb_;
    double coef_b_;
    double coef_d_;
    double std_dev_;
    double salt_;
    double pepper_;
    bool flip_;
    FilterType filterType_;
};

}

#endif
