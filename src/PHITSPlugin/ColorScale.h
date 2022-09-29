/**
   \file
   \author Kenta Suzuki
*/

#ifndef CNOID_PHITSPLUGIN_COLORSCALE_H
#define CNOID_PHITSPLUGIN_COLORSCALE_H

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

#endif // CNOID_PHITSPLUGIN_COLORSCALE_H
