/**
   @author Kenta Suzuki
*/

#ifndef CNOID_JOYSTICKSTART_PLUGIN_AXIS_WIDGET_H
#define CNOID_JOYSTICKSTART_PLUGIN_AXIS_WIDGET_H

#include <cnoid/Signal>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QWidget>

namespace cnoid {

class AxisWidget : public QWidget
{
public:
    AxisWidget(QWidget* parent = nullptr);
    virtual ~AxisWidget();

    void setValue(const int& id, const double& value);

    SignalProxy<void(const double& h_position, const double& v_position)> sigAxis();

protected:
    virtual void paintEvent(QPaintEvent* event) override;
    virtual void mousePressEvent(QMouseEvent* event) override;
    virtual void mouseReleaseEvent(QMouseEvent* event) override;
    virtual void mouseMoveEvent(QMouseEvent* event) override;

private:
    class Impl;
    Impl* impl;
};

}

#endif // CNOID_JOYSTICKSTART_PLUGIN_AXIS_WIDGET_H