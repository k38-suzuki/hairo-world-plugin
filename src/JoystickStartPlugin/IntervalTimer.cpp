/**
   @author Kenta Suzuki
*/

#include "IntervalTimer.h"
#include <cnoid/Archive>
#include <cnoid/Buttons>
#include <cnoid/Dialog>
#include <cnoid/ExtensionManager>
#include <cnoid/Format>
#include <cnoid/MainMenu>
#include <cnoid/MessageView>
#include <cnoid/Separator>
#include <cnoid/SimulationBar>
#include <cnoid/SimulatorItem>
#include <cnoid/SpinBox>
#include <cnoid/TimeBar>
#include <cnoid/Timer>
#include <QBoxLayout>
#include <QDialogButtonBox>
#include <QLabel>
#include <QLCDNumber>
#include <QTime>
#include "gettext.h"

using namespace std;
using namespace cnoid;

namespace {

class TimerDialog : public QDialog
{
public:
    TimerDialog(QWidget* parent = nullptr);

private:
    void onCountdown();
    void onTimeout();
    void onButtonToggled(bool checked);
    void onSimulationAboutToStart(SimulatorItem* simulatorItem);
    void onPlaybackStopped(double time, bool isStoppedManually);
    void onTimeChanged(double time);

    SpinBox* intervalSpinBox;

    SimulationBar* sb;
    SimulatorItem* simulatorItem;
    TimeBar* tb;
    Timer* startTimer;
    Timer* intervalTimer;
    PushButton* startButton;

    bool is_simulation_started;
    int counter;

    QLCDNumber* lcdNumber;
    QDialogButtonBox* buttonBox;
};

}


void IntervalTimer::initializeClass(ExtensionManager* ext)
{
    static TimerDialog* dialog = nullptr;

    if(!dialog) {
        dialog = ext->manage(new TimerDialog);

        MainMenu::instance()->add_Tools_Item(
            _("Interval Timer"), [](){ dialog->show(); });
    }
}


IntervalTimer::IntervalTimer()
{

}


IntervalTimer::~IntervalTimer()
{

}


TimerDialog::TimerDialog(QWidget* parent)
    : QDialog(parent),
      sb(SimulationBar::instance()),
      tb(TimeBar::instance()),
      is_simulation_started(false),
      counter(5)
{
    setMinimumSize(320, 240);

    // interval timer
    intervalSpinBox = new SpinBox;
    intervalSpinBox->setValue(counter);

    startTimer = new Timer(tb);
    startTimer->sigTimeout().connect([&](){ onCountdown(); });

    intervalTimer = new Timer(tb);
    intervalTimer->sigTimeout().connect([&](){ onTimeout(); });

    tb->addWidget(intervalSpinBox);
    tb->sigPlaybackStopped().connect(
        [&](double time, bool isStoppedManually){ onPlaybackStopped(time, isStoppedManually); });    

    const QIcon startIcon = QIcon("media-playback-start");
    startButton = new PushButton(startIcon, _("Start"));
    startButton->setCheckable(true);
    startButton->sigToggled().connect([&](bool checked){ onButtonToggled(checked); });

    sb->sigSimulationAboutToStart().connect(
        [&](SimulatorItem* simulatorItem){ onSimulationAboutToStart(simulatorItem); });

    // digital clock
    lcdNumber = new QLCDNumber(this);
    lcdNumber->setSegmentStyle(QLCDNumber::Filled);

    TimeBar::instance()->sigTimeChanged().connect(
        [&](double time){ onTimeChanged(time); return true; });

    onTimeChanged(0.0);

    auto layout = new QHBoxLayout;
    layout->addWidget(new QLabel(_("Interval [s]")));
    layout->addWidget(intervalSpinBox);
    layout->addWidget(startButton);
    // layout->addStretch();

    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok);
    connect(buttonBox, &QDialogButtonBox::accepted, [&](){ accept(); });

    auto mainLayout = new QVBoxLayout;
    mainLayout->addLayout(layout);
    mainLayout->addWidget(lcdNumber);
    mainLayout->addWidget(new HSeparator);
    mainLayout->addWidget(buttonBox);
    setLayout(mainLayout);

    setWindowTitle(_("Interval Timer"));
}


void TimerDialog::onCountdown()
{
    if(counter > 0) {
        MessageView::instance()->putln(formatR(_("{0}"), counter));
        --counter;
    } else {
        MessageView::instance()->putln(formatR(_("Start!!")));
        counter = intervalSpinBox->value();
        sb->startSimulation(true);
    }
}


void TimerDialog::onTimeout()
{
    intervalTimer->stop();
    onButtonToggled(true);
}


void TimerDialog::onButtonToggled(bool checked)
{
    if(checked) {
        counter = intervalSpinBox->value();
        startTimer->start(1000);
    } else {
        if(startTimer->isActive()) {
            startTimer->stop();
        }
    }
}


void TimerDialog::onSimulationAboutToStart(SimulatorItem* simulatorItem)
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


void TimerDialog::onPlaybackStopped(double time, bool isStoppedManually)
{
    bool is_starter_checked = startButton->isChecked();
    if(is_simulation_started && is_starter_checked) {
        intervalTimer->start(2000);
    }
    is_simulation_started = false;
}


void TimerDialog::onTimeChanged(double time)
{
    int minute = time / 60.0;
    int second = time - minute * 60.0;
    QTime currentTime(0, minute, second, 0);
    QString text = currentTime.toString("mm:ss");
    // if((currentTime.second() % 2) == 0) {
    //     text[2] = ' ';
    // }
    lcdNumber->display(text);
}