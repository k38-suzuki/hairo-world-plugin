/**
   @author Kenta Suzuki
*/

#ifndef CNOID_BODY_CREATOR_PLUGIN_WIDGET_INFO_H
#define CNOID_BODY_CREATOR_PLUGIN_WIDGET_INFO_H

namespace cnoid {

struct DoubleSpinInfo
{
    int row;
    int column;
    double min;
    double max;
    double step;
    double decimals;
    double value;
    const char* key;
    DoubleSpinBox* spin;
};

struct SpinInfo
{
    int row;
    int column;
    int min;
    int max;
    int step;
    int value;
    const char* key;
    SpinBox* spin;
};

}

#endif // CNOID_BODY_CREATOR_PLUGIN_WIDGET_INFO_H