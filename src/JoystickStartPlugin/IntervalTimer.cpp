/**
   @author Kenta Suzuki
*/

#include "IntervalTimer.h"
#include <cnoid/Archive>
#include <cnoid/Buttons>
#include <cnoid/ExtensionManager>
#include <cnoid/Format>
#include <cnoid/MessageView>
#include <cnoid/SimulationBar>
#include <cnoid/SimulatorItem>
#include <cnoid/SpinBox>
#include <cnoid/TimeBar>
#include <cnoid/Timer>
#include "gettext.h"

using namespace std;
using namespace cnoid;

namespace {

IntervalTimer* timerInstance = nullptr;

}

namespace cnoid {

class IntervalTimer::Impl
{
public:

    Impl();
    ~Impl();

    void onCountdown();
    void onTimeout();
    void onButtonToggled(bool checked);
    void onSimulationAboutToStart(SimulatorItem* simulatorItem);
    void onPlaybackStopped(double time, bool isStoppedManually);

    SpinBox* intervalSpin;

    SimulationBar* sb;
    SimulatorItem* simulatorItem;
    TimeBar* tb;
    Timer* startTimer;
    Timer* intervalTimer;
    ToolButton* startButton;

    bool is_simulation_started;
    int counter;
};

}


void IntervalTimer::initializeClass(ExtensionManager* ext)
{
    if(!timerInstance) {
        timerInstance = ext->manage(new IntervalTimer);
    }
}


IntervalTimer::IntervalTimer()
{
    impl = new Impl;
}


IntervalTimer::Impl::Impl()
    : sb(SimulationBar::instance()),
      tb(TimeBar::instance())
{
    is_simulation_started = false;
    counter = 5;

    intervalSpin = new SpinBox;
    intervalSpin->setValue(counter);
    intervalSpin->setToolTip(_("Interval time"));

    startTimer = new Timer(tb);
    startTimer->sigTimeout().connect([&](){ onCountdown(); });

    intervalTimer = new Timer(tb);
    intervalTimer->sigTimeout().connect([&](){ onTimeout(); });

    tb->addWidget(intervalSpin);
    tb->sigPlaybackStopped().connect(
        [&](double time, bool isStoppedManually){ onPlaybackStopped(time, isStoppedManually); });    

    startButton = tb->addToggleButton(":/GoogleMaterialSymbols/icon/timer_play_24dp_5F6368_FILL1_wght400_GRAD0_opsz24.svg");
    startButton->setToolTip(_("Set the interval timer"));
    startButton->sigToggled().connect([&](bool checked){ onButtonToggled(checked); });

    sb->sigSimulationAboutToStart().connect(
            [&](SimulatorItem* simulatorItem){ onSimulationAboutToStart(simulatorItem); });
}


IntervalTimer::~IntervalTimer()
{
    delete impl;
}


IntervalTimer::Impl::~Impl()
{

}


void IntervalTimer::Impl::onCountdown()
{
    if(counter > 0) {
        MessageView::instance()->putln(formatR(_("{0}"), counter));
        --counter;
    } else {
        MessageView::instance()->putln(formatR(_("Start!!")));
        counter = intervalSpin->value();
        sb->startSimulation(true);
    }
}


void IntervalTimer::Impl::onTimeout()
{
    intervalTimer->stop();
    onButtonToggled(true);
}


void IntervalTimer::Impl::onButtonToggled(bool checked)
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


void IntervalTimer::Impl::onSimulationAboutToStart(SimulatorItem* simulatorItem)
{
    if(startTimer->isActive()) {
        startTimer->stop();
    }

    if(intervalTimer->isActive()) {
        intervalTimer->stop();
    }

    this->simulatorItem = simulatorItem;
    is_simulation_started = true;
}


void IntervalTimer::Impl::onPlaybackStopped(double time, bool isStoppedManually)
{
    bool is_starter_checked = startButton->isChecked();
    if(is_simulation_started && is_starter_checked) {
        intervalTimer->start(2000);
    }
    is_simulation_started = false;
}