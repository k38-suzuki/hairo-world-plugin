/**
    @author Kenta Suzuki
*/

#include "DigitalClock.h"
#include <cnoid/Action>
#include <cnoid/ExtensionManager>
#include <cnoid/HamburgerMenu>
#include <cnoid/TimeBar>
#include <QTime>
#include "gettext.h"

using namespace std;
using namespace cnoid;

namespace {

DigitalClock* clockInstance = nullptr;

}

namespace cnoid {

class DigitalClock::Impl
{
public:
    DigitalClock* self;

    Impl(DigitalClock* self);
    ~Impl();

    void showTime(const double& time);
};

}


void DigitalClock::initializeClass(ExtensionManager* ext)
{
    if(!clockInstance) {
        clockInstance = ext->manage(new DigitalClock);

        const QIcon icon = QIcon(":/GoogleMaterialSymbols/icon/alarm_24dp_5F6368_FILL1_wght400_GRAD0_opsz24.svg");
        auto action = new Action;
        action->setText(_("Digital Clock"));
        action->setIcon(icon);
        action->setToolTip(_("Show the digital clock"));
        action->sigTriggered().connect([&](){ clockInstance->show(); });
        HamburgerMenu::instance()->addAction(action);
    }
}


DigitalClock::DigitalClock(QWidget* parent)
    : QLCDNumber(parent)
{
    impl = new Impl(this);
}


DigitalClock::Impl::Impl(DigitalClock* self)
    : self(self)
{
    self->setSegmentStyle(Filled);
    self->setWindowFlags(Qt::WindowStaysOnTopHint);

    TimeBar::instance()->sigTimeChanged().connect(
        [&](double time){ showTime(time); return true; });

    showTime(0.0);

    self->setWindowTitle(_("Digital Clock"));
    self->resize(150, 60);
}


DigitalClock::~DigitalClock()
{
    delete impl;
}


DigitalClock::Impl::~Impl()
{

}


void DigitalClock::Impl::showTime(const double& time)
{
    int minute = time / 60.0;
    int second = time - minute * 60.0;
    QTime currentTime(0, minute, second, 0);
    QString text = currentTime.toString("mm:ss");
    // if((currentTime.second() % 2) == 0) {
    //     text[2] = ' ';
    // }
    self->display(text);
}