/**
   @author Kenta Suzuki
*/

#include "ColorScale.h"
#include <QColor>
#include <math.h>

using namespace cnoid;


ColorScale::ColorScale()
{
    min_ = 0.0;
    max_ = 0.0;
    color_ << 1.0, 1.0, 1.0;
}


ColorScale::~ColorScale()
{

}


void ColorScale::setRange(const double& min, const double& max)
{
    min_ = min;
    max_ = max;
}


Vector3& ColorScale::linerColor(const double& value)
{
    double min = min_;
    double max = max_;
    double val = value;

    if(val < min) {
        val = min;
    } else if(val > max) {
        val = max;
    }

    scaledColor(min, max, val);
    return color_;
}


Vector3& ColorScale::logColor(const double& value)
{
    double min = log10(min_);
    double max = log10(max_);
    double val = log10(value);

    if(isinf(max) || isinf(val) || (val < min)) {
        return color_;
    }

    scaledColor(min, max, val);
    return color_;
}


void ColorScale::scaledColor(const double& min, const double& max, const double& value)
{
    double hue = 240.0;
    if((max - min) > 0.0) {
        hue = (1.0 - (value - min) / (max - min)) * 240.0;
    }

    QColor color = QColor::fromHsv((int)hue, 255, 255);
    double red = (double)color.red() / 255.0;
    double green = (double)color.green() / 255.0;
    double blue = (double)color.blue() / 255.0;
    color_ << red, green, blue;
}
