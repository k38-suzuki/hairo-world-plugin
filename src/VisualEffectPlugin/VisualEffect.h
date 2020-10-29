/**
   \file
   \author Kenta Suzuki
*/

#ifndef CNOID_VISUAL_EFFECT_PLUGIN_VISUAL_EFFECT_H
#define CNOID_VISUAL_EFFECT_PLUGIN_VISUAL_EFFECT_H

namespace cnoid {

class VisualEffectImpl;

class VisualEffect
{
public:
    VisualEffect();
    virtual ~VisualEffect();

    void setHue(const double hue);
    double hue() const;
    void setSaturation(const double saturation);
    double saturation() const;
    void setValue(const double value);
    double value() const;
    void setRed(const double red);
    double red() const;
    void setGreen(const double green);
    double green() const;
    void setBlue(const double blue);
    double blue() const;
    void setCoefB(const double coefB);
    double coefB() const;
    void setCoefD(const double coefD);
    double coefD() const;
    void setStdDev(const double stdDev);
    double stdDev() const;
    void setSalt(const double salt);
    double salt() const;
    void setPepper(const double pepper);
    double pepper() const;

private:
    VisualEffectImpl* impl;
    friend class VisualEffectImpl;
};

}

#endif // CNOID_VISUAL_EFFECT_PLUGIN_VISUAL_EFFECT_H
