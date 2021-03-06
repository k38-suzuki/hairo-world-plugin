/**
   \file
   \author Kenta Suzuki
*/

#ifndef CNOID_VISUAL_EFFECT_PLUGIN_VISUAL_EFFECT_H
#define CNOID_VISUAL_EFFECT_PLUGIN_VISUAL_EFFECT_H

namespace cnoid {

class VisualEffect
{
public:
    VisualEffect();
    virtual ~VisualEffect();

    void setHue(const double& hue) { hue_ = hue; }
    double hue() const { return hue_; }
    void setSaturation(const double& saturation) { saturation_ = saturation; }
    double saturation() const { return saturation_; }
    void setValue(const double& value) { value_ = value; }
    double value() const { return value_; }
    void setRed(const double& red) { red_ = red; }
    double red() const { return red_; }
    void setGreen(const double& green) { green_ = green; }
    double green() const { return green_; }
    void setBlue(const double& blue) { blue_ = blue; }
    double blue() const { return blue_; }
    void setCoefB(const double& coefB) { coefB_ = coefB; }
    double coefB() const { return coefB_; }
    void setCoefD(const double& coefD) { coefD_ = coefD; }
    double coefD() const { return coefD_; }
    void setStdDev(const double& stdDev) { stdDev_ = stdDev; }
    double stdDev() const { return stdDev_; }
    void setSalt(const double& salt) { salt_ = salt; }
    double salt() const { return salt_; }
    void setPepper(const double& pepper) { pepper_ = pepper; }
    double pepper() const { return pepper_; }
    void setFlip(const bool& flip) { flip_ = flip; }
    bool flip() const { return flip_; }


private:
    double hue_;
    double saturation_;
    double value_;
    double red_;
    double green_;
    double blue_;
    double coefB_;
    double coefD_;
    double stdDev_;
    double salt_;
    double pepper_;
    bool flip_;
};

}

#endif // CNOID_VISUAL_EFFECT_PLUGIN_VISUAL_EFFECT_H
