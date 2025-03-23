/**
   @author Kenta Suzuki
*/

#include "AxisWidget.h"
#include <cnoid/Separator>
#include <QBoxLayout>
#include <QGridLayout>
#include <QPainter>

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

class AxisWidget::Impl
{
public:
    AxisWidget* self;

    Impl(AxisWidget* self);

    bool isLeftButtonPressed;
    QPoint pos;
    QPoint org;
    double joy[2];
    int radius;

    Signal<void(const double& h_position, const double& v_position)> sigAxis;

    void paintEvent(QPaintEvent* event);
    void mousePressEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);
};

}


AxisWidget::AxisWidget(QWidget* parent)
    : QWidget(parent)
{
    impl = new Impl(this);
}


AxisWidget::Impl::Impl(AxisWidget *self)
    : self(self)
{
    joy[0] = joy[1] = 0.0;
    isLeftButtonPressed = false;

    self->setFixedSize(100, 100);
    pos = org = QPoint(self->width() / 2, self->height() / 2);
    radius = 5;
    self->update();

    auto gridLayout = new QGridLayout;
    gridLayout->addWidget(new VSeparator, 0, 1);
    gridLayout->addWidget(new VSeparator, 2, 1);
    gridLayout->addWidget(new HSeparator, 1, 0);
    gridLayout->addWidget(new HSeparator, 1, 2);

    auto vbox = new QVBoxLayout;
    vbox->addLayout(gridLayout);
    self->setLayout(vbox);
}


AxisWidget::~AxisWidget()
{
    delete impl;
}


void AxisWidget::setValue(const int& id, const double& value)
{
    double r = (value + 1.0) / 2.0;
    if(id == 0) {
        impl->pos.setX(r * width());
    } else if(id == 1) {
        impl->pos.setY(r * height());
    }
    update();
}


SignalProxy<void(const double& h_position, const double& v_position)> AxisWidget::sigAxis()
{
    return impl->sigAxis;
}


void AxisWidget::paintEvent(QPaintEvent* event)
{
    impl->paintEvent(event);
}


void AxisWidget::Impl::paintEvent(QPaintEvent* event)
{
    QPainter painter(self);
    painter.setPen(Qt::black);
    painter.setBrush(Qt::red);
    painter.drawEllipse(pos.x() - radius, pos.y() - radius, radius * 2, radius * 2);
}


void AxisWidget::mousePressEvent(QMouseEvent* event)
{
    impl->mousePressEvent(event);
}


void AxisWidget::Impl::mousePressEvent(QMouseEvent* event)
{
    if(event->button() == Qt::LeftButton) {
        isLeftButtonPressed = true;
        sigAxis(0.0, 0.0);
        pos = org;
        self->update();
    }
}


void AxisWidget::mouseReleaseEvent(QMouseEvent* event)
{
    impl->mouseReleaseEvent(event);
}


void AxisWidget::Impl::mouseReleaseEvent(QMouseEvent* event)
{
    if(event->button() == Qt::LeftButton) {
        isLeftButtonPressed = false;
        sigAxis(0.0, 0.0);
        pos = org;
        self->update();
    }
}


void AxisWidget::mouseMoveEvent(QMouseEvent* event)
{
    impl->mouseMoveEvent(event);
}


void AxisWidget::Impl::mouseMoveEvent(QMouseEvent* event)
{
    if(isLeftButtonPressed) {
        int w = self->width();
        int h = self->height();
        int x = event->pos().x();
        int y = event->pos().y();
        joy[0] = (double)x / (double)w;
        joy[1] = (double)y / (double)h;
        for(int i = 0; i < 2; ++i) {
            onMouseMoved(joy[i]);
        }
        sigAxis(joy[0], joy[1]);
        pos = QPoint(x, y);
        org = QPoint(w / 2, h / 2);
        self->update();
    }
}