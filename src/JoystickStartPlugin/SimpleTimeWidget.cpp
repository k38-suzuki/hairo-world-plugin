/**
   @author Kenta Suzuki
*/

#include "SimpleTimeWidget.h"
#include <cnoid/Slider>
#include <cnoid/SpinBox>
#include <cnoid/TimeBar>
#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>
#include <cmath>
#include "gettext.h"

using namespace cnoid;

namespace cnoid {

class SimpleTimeWidgetImpl
{
public:
    SimpleTimeWidgetImpl(SimpleTimeWidget* self);
    SimpleTimeWidget* self;

    DoubleSpinBox* timeDSpin;
    Slider* timeSlider;
    TimeBar* tb;
    int decimals;
    double minTime;
    double maxTime;

    void onTimeChanged(const double& time);
    void onValueChanged(const double& value);
    void onValueChanged(const int& value);
};

}


SimpleTimeWidget::SimpleTimeWidget()
{
    impl = new SimpleTimeWidgetImpl(this);
}


SimpleTimeWidgetImpl::SimpleTimeWidgetImpl(SimpleTimeWidget* self)
    : self(self),
      tb(TimeBar::instance())
{
    decimals = 2;
    minTime = 0.0;
    maxTime = 0.0;

    timeDSpin = new DoubleSpinBox;
    timeDSpin->setDecimals(3);
    timeDSpin->setRange(0.0, 9999.999);
    timeDSpin->setValue(0.0);

    timeSlider = new Slider(Qt::Horizontal);
    timeSlider->setValue(0);

    QHBoxLayout* hbox = new QHBoxLayout;
    hbox->addWidget(new QLabel(_("Time")));
    hbox->addWidget(timeSlider);
    hbox->addWidget(timeDSpin);

    QVBoxLayout* vbox = new QVBoxLayout;
    vbox->addLayout(hbox);
    self->setLayout(vbox);

    tb->sigTimeChanged().connect([&](double time){ onTimeChanged(time); return true; });
    timeDSpin->sigValueChanged().connect([&](double value){ onValueChanged(value); });
    timeSlider->sigValueChanged().connect([&](int value){ onValueChanged(value); });
}


SimpleTimeWidget::~SimpleTimeWidget()
{
    delete impl;
}


void SimpleTimeWidgetImpl::onTimeChanged(const double& time)
{
    const double timeStep = 1.0 / tb->frameRate();
    decimals = static_cast<int>(ceil(log10(tb->frameRate())));
    const double r = pow(10.0, decimals);
    minTime = tb->minTime();
    maxTime = tb->maxTime();
    timeDSpin->blockSignals(true);
    timeDSpin->setRange(minTime, maxTime);
    timeDSpin->setDecimals(decimals);
    timeDSpin->setSingleStep(timeStep);
    timeDSpin->setValue(time);
    timeDSpin->blockSignals(false);

    timeSlider->blockSignals(true);
    timeSlider->setRange((int)nearbyint(minTime * r), (int)nearbyint(maxTime * r));
    timeSlider->setSingleStep(timeStep * r);
    timeSlider->setValue((int)nearbyint(time * pow(10.0, decimals)));
    timeSlider->blockSignals(false);
}


void SimpleTimeWidgetImpl::onValueChanged(const double& value)
{
    if(tb->isDoingPlayback()) {
        tb->stopPlayback(true);
    }
    tb->setTime(value);
}


void SimpleTimeWidgetImpl::onValueChanged(const int& value)
{
    if(tb->isDoingPlayback()) {
        tb->stopPlayback(true);
    }
    tb->setTime(value / pow(10.0, decimals));
}
