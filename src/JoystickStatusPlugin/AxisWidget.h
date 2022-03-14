/**
   \file
   \author Kenta Suzuki
*/

#ifndef CNOID_JOYSTICKSTATUSPLUGIN_AXISWIDGET_H
#define CNOID_JOYSTICKSTATUSPLUGIN_AXISWIDGET_H

#include <cnoid/Signal>
#include <cnoid/Widget>
#include <QMouseEvent>
#include <QPaintEvent>

namespace cnoid {

class AxisWidgetImpl;

class AxisWidget : public Widget
{
public:
    AxisWidget();
    virtual ~AxisWidget();

    void setValue(const int& id, const double& value);

    SignalProxy<void(double h_position, double v_position)> sigAxis();

protected:
    virtual void paintEvent(QPaintEvent* event) override;
    virtual void mousePressEvent(QMouseEvent* event) override;
    virtual void mouseReleaseEvent(QMouseEvent* event) override;
    virtual void mouseMoveEvent(QMouseEvent* event) override;

private:
    AxisWidgetImpl* impl;
    friend class AxisWidgetImpl;
};

}

#endif // CNOID_JOYSTICKSTATUSPLUGIN_AXISWIDGET_H
