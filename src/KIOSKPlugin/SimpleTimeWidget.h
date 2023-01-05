/**
   \file
   \author Kenta Suzuki
*/

#ifndef CNOID_KIOSK_PLUGIN_SIMPLE_TIME_WIDGET_H
#define CNOID_KIOSK_PLUGIN_SIMPLE_TIME_WIDGET_H

#include <cnoid/Widget>

namespace cnoid {

class SimpleTimeWidgetImpl;

class SimpleTimeWidget : public Widget
{
public:
    SimpleTimeWidget();
    virtual ~SimpleTimeWidget();

private:
    SimpleTimeWidgetImpl* impl;
    friend class SimpleTimeWidgetImpl;
};

}

#endif // CNOID_KIOSK_PLUGIN_SIMPLE_TIME_WIDGET_H