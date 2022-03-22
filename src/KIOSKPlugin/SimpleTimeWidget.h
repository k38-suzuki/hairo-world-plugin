/**
   \file
   \author Kenta Suzuki
*/

#ifndef CNOID_KIOSKPLUGIN_SIMPLETIMEWIDGET_H
#define CNOID_KIOSKPLUGIN_SIMPLETIMEWIDGET_H

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

#endif // CNOID_KIOSKPLUGIN_SIMPLETIMEWIDGET_H
