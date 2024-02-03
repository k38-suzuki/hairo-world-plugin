/**
   @author Kenta Suzuki
*/

#ifndef CNOID_JOYSTICK_START_PLUGIN_SIMPLE_TIME_WIDGET_H
#define CNOID_JOYSTICK_START_PLUGIN_SIMPLE_TIME_WIDGET_H

#include <cnoid/Widget>

namespace cnoid {

class SimpleTimeWidget : public Widget
{
public:
    SimpleTimeWidget();
    virtual ~SimpleTimeWidget();

private:
    class Impl;
    Impl* impl;
};

}

#endif
