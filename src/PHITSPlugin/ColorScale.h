/**
   @author Kenta Suzuki
*/

#ifndef CNOID_PHITS_PLUGIN_COLOR_SCALE_H
#define CNOID_PHITS_PLUGIN_COLOR_SCALE_H

#include <cnoid/EigenTypes>

namespace cnoid {

class ColorScale
{
public:
    ColorScale();
    virtual ~ColorScale();

    void setRange(const double& min, const double& max);
    Vector3& linerColor(const double& value);
    Vector3& logColor(const double& value);

private:
    double min_;
    double max_;
    Vector3 color_;
    void scaledColor(const double& min, const double& max, const double& value);
};

}

#endif
