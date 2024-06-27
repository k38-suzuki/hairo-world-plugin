/**
   @author Kenta Suzuki
*/

#include "IntervalStarterBar.h"
#include <cnoid/ExtensionManager>
#include <cnoid/MessageView>
#include <cnoid/SimulationBar>
#include <cnoid/SimulatorItem>
#include <cnoid/SpinBox>
#include <cnoid/TimeBar>
#include <cnoid/Timer>
#include <QLabel>
#include <fmt/format.h>
#include "gettext.h"

using namespace std;
using namespace cnoid;
using fmt::format;

namespace cnoid {

class IntervalStarterBar::Impl
{
public:
    IntervalStarterBar* self;

    Impl(IntervalStarterBar* self);
    ~Impl();

    SpinBox* intervalSpin;

    SimulationBar* sb;
    SimulatorItem* simulatorItem;
    Timer* startTimer;
    Timer* intervalTimer;
    ToolButton* startButton;

    bool is_simulation_started;
    int counter;

    void onCountdown();
    void onTimeout();
    void onButtonToggled(bool checked);
    void onSimulationAboutToStart(SimulatorItem* simulatorItem);
    void onPlaybackStopped(double time, bool isStoppedManually);
};

}


void IntervalStarterBar::initialize(ExtensionManager* ext)
{
    static bool initialized = false;
    if(!initialized) {
        // ext->addToolBar(instance());
        instance();
        initialized = true;
    }
}


IntervalStarterBar* IntervalStarterBar::instance()
{
    static IntervalStarterBar* starterBar = new IntervalStarterBar;
    return starterBar;
}


IntervalStarterBar::IntervalStarterBar()
    : ToolBar(N_("IntervalStarterBar"))
{
    impl = new Impl(this);
}


IntervalStarterBar::Impl::Impl(IntervalStarterBar* self)
    : self(self),
      sb(SimulationBar::instance())
{
    self->setVisibleByDefault(false);

    is_simulation_started = false;
    counter = 5;

    intervalSpin = new SpinBox;
    intervalSpin->setValue(counter);
    intervalSpin->setToolTip(_("Interval time"));
    sb->addWidget(new QLabel(" : "));
    sb->addWidget(intervalSpin);

    startTimer = new Timer(self);
    startTimer->sigTimeout().connect([&](){ onCountdown(); });

    intervalTimer = new Timer(self);
    intervalTimer->sigTimeout().connect([&](){ onTimeout(); });

    const QIcon startIcon = QIcon::fromTheme("media-playlist-repeat");
    startButton = sb->addToggleButton(startIcon);
    startButton->setToolTip(_("Set the interval timer"));
    startButton->sigToggled().connect([&](bool checked){ onButtonToggled(checked); });

    sb->sigSimulationAboutToStart().connect(
            [&](SimulatorItem* simulatorItem){ onSimulationAboutToStart(simulatorItem); });

    TimeBar::instance()->sigPlaybackStopped().connect(
        [&](double time, bool isStoppedManually){ onPlaybackStopped(time, isStoppedManually); });
}


IntervalStarterBar::~IntervalStarterBar()
{
    delete impl;
}


IntervalStarterBar::Impl::~Impl()
{

}


void IntervalStarterBar::Impl::onCountdown()
{
    if(counter > 0) {
        MessageView::instance()->putln(format(_("{0}"), counter));
        --counter;
    } else {
        MessageView::instance()->putln(format(_("Start!!")));
        counter = intervalSpin->value();
        startTimer->stop();
        sb->startSimulation(true);
    }
}


void IntervalStarterBar::Impl::onTimeout()
{
    intervalTimer->stop();
    onButtonToggled(true);
}


void IntervalStarterBar::Impl::onButtonToggled(bool checked)
{
    if(checked) {
        counter = intervalSpin->value();
        startTimer->start(1000);
    } else {
        if(startTimer->isActive()) {
            startTimer->stop();
        }
    }
}


void IntervalStarterBar::Impl::onSimulationAboutToStart(SimulatorItem* simulatorItem)
{
    this->simulatorItem = simulatorItem;
    is_simulation_started = true;
}


void IntervalStarterBar::Impl::onPlaybackStopped(double time, bool isStoppedManually)
{
    bool is_starter_checked = startButton->isChecked();
    if(is_simulation_started && is_starter_checked) {
        intervalTimer->start(2000);
    }
    is_simulation_started = false;
}