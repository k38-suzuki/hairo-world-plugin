/**
   \file
   \author Kenta Suzuki
*/

#include "AxisWidget.h"
#include <cnoid/Separator>
#include <QGridLayout>
#include <QVBoxLayout>

using namespace cnoid;

namespace {

void onMouseMoved(double& value)
{
    if(value > 1.0) {
        value = 1.0;
    } else if(value < 0.0) {
        value = 0.0;
    }

    value = (value - 0.5) * 2.0;
}

}


namespace cnoid {

class AxisWidgetImpl
{
public:
    AxisWidgetImpl(AxisWidget* self);
    AxisWidget* self;

    double h_position;
    double v_position;
    bool isLeftButtonPressed;

    Signal<void(double h_position, double v_position)> sigAxis;

    void mousePressEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);
};

}


AxisWidget::AxisWidget()
{
    impl = new AxisWidgetImpl(this);
}


AxisWidgetImpl::AxisWidgetImpl(AxisWidget *self)
    : self(self)
{
    h_position = 0.0;
    v_position = 0.0;
    isLeftButtonPressed = false;

    self->setFixedSize(100, 100);
    QGridLayout* gbox = new QGridLayout;
    gbox->addWidget(new VSeparator, 0, 1);
    gbox->addWidget(new VSeparator, 2, 1);
    gbox->addWidget(new HSeparator, 1, 0);
    gbox->addWidget(new HSeparator, 1, 2);

    QVBoxLayout* vbox = new QVBoxLayout;
    vbox->addLayout(gbox);
    self->setLayout(vbox);
}


AxisWidget::~AxisWidget()
{
    delete impl;
}


SignalProxy<void(double h_position, double v_position)> AxisWidget::sigAxis()
{
    return impl->sigAxis;
}


void AxisWidget::mousePressEvent(QMouseEvent* event)
{
    impl->mousePressEvent(event);
}


void AxisWidgetImpl::mousePressEvent(QMouseEvent* event)
{
    if(event->button() == Qt::LeftButton) {
        isLeftButtonPressed = true;
        sigAxis(0.0, 0.0);
    }
}


void AxisWidget::mouseReleaseEvent(QMouseEvent* event)
{
    impl->mouseReleaseEvent(event);
}


void AxisWidgetImpl::mouseReleaseEvent(QMouseEvent* event)
{
    if(event->button() == Qt::LeftButton) {
        isLeftButtonPressed = false;
        sigAxis(0.0, 0.0);
    }
}


void AxisWidget::mouseMoveEvent(QMouseEvent* event)
{
    impl->mouseMoveEvent(event);
}


void AxisWidgetImpl::mouseMoveEvent(QMouseEvent* event)
{
    if(isLeftButtonPressed) {
        int w = self->width();
        int h = self->height();
        int x = event->pos().x();
        int y = event->pos().y();
        h_position = (double)x / (double)w;
        v_position = (double)y / (double)h;
        onMouseMoved(h_position);
        onMouseMoved(v_position);
        sigAxis(h_position, v_position);
    }
}
