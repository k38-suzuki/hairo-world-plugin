/**
   \file
   \author Kenta Suzuki
*/

#include "SimpleSimulationView.h"
#include <cnoid/Button>
#include <cnoid/MainWindow>
#include <cnoid/SimulationBar>
#include <cnoid/SimulatorItem>
#include <cnoid/Slider>
#include <cnoid/SpinBox>
#include <cnoid/TimeBar>
#include <cnoid/ViewManager>
#include <cnoid/Widget>
#include <QGridLayout>
#include <QLabel>
#include <QScrollArea>
#include <QStyle>
#include <QVBoxLayout>
#include "gettext.h"

using namespace cnoid;

namespace {

struct ButtonState {
    bool start;
    bool restart;
    bool pause;
    bool stop;
};

ButtonState buttonStates[] = {
    { false, false,  true,  true },
    { false, false,  true,  true },
    { false, false,  true,  true },
    {  true,  true, false, false }
};

struct ButtonInfo {
    int row;
    int column;
    int rowSpan;
    int columnSpan;
    bool enabled;
    const char* icon;
};

ButtonInfo buttonInfo[] = {
    { 0, 0, 1, 1,  true,   ":/Body/icon/start-simulation.svg" },
    { 0, 1, 1, 1,  true, ":/Body/icon/restart-simulation.svg" },
    { 0, 2, 1, 1, false,   ":/Body/icon/pause-simulation.svg" },
    { 0, 3, 1, 1, false,    ":/Body/icon/stop-simulation.svg" },
    { 2, 2, 1, 1,  true,                                   "" },
    { 2, 3, 1, 1,  true,                                   "" },
    { 3, 0, 1, 1,  true,               ":/Base/icon/play.svg" },
    { 3, 1, 1, 1,  true,             ":/Base/icon/resume.svg" }
};

}


namespace cnoid {

class SimpleSimulationViewImpl
{
public:
    SimpleSimulationViewImpl(SimpleSimulationView* self);
    SimpleSimulationView* self;

    enum ButtonID {
        START, RESTART, PAUSE, STOP,
        BACKWARD, FORWARD, PLAY, RESUME,
        NUM_BUTTONS
    };

    enum DoubleSpinID { SEEKTIME, TIME, NUM_DSPINS };

    QScrollArea scrollArea;

    PushButton* buttons[NUM_BUTTONS];
    DoubleSpinBox* dspins[NUM_DSPINS];
    Slider* timeSlider;
    SimulatorItem* simulatorItem;
    SimulationBar* sb;
    TimeBar* tb;
    int decimals;
    double minTime;
    double maxTime;
    QIcon resumeIcon;
    QIcon stopIcon;

    void onSimulationAboutToStart(SimulatorItem* simulatorItem);
    void onButtonClicked(const int& id);
    void onValueChanged(const double& value);
    void onValueChanged(const int& value);
    void onTimeChanged(const double& time);
    void onPlaybackStarted(const double& time);
    void onPlaybackStopped(const double& time, const bool& isStoppedManually);
    void updateButtonStates(const int& state);
};

}


SimpleSimulationView::SimpleSimulationView()
{
    impl = new SimpleSimulationViewImpl(this);
}


SimpleSimulationViewImpl::SimpleSimulationViewImpl(SimpleSimulationView* self)
    : self(self),
      sb(SimulationBar::instance()),
      tb(TimeBar::instance()),
      resumeIcon(QIcon(":/Base/icon/resume.svg")),
      stopIcon(QIcon(":/Base/icon/stop.svg"))
{
    simulatorItem = nullptr;
    decimals = 2;
    minTime = 0.0;
    maxTime = 0.0;
    self->setDefaultLayoutArea(View::TopRightArea);

    QWidget* topWidget = new QWidget;
    topWidget->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);

    QVBoxLayout* topVBox = new QVBoxLayout;
    //topVBox->setContentsMargins(4);
    topWidget->setLayout(topVBox);

    scrollArea.setStyleSheet("QScrollArea {background: transparent;}");
    scrollArea.setFrameShape(QFrame::NoFrame);
    scrollArea.setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea.setWidget(topWidget);
    topWidget->setAutoFillBackground(false);
    QVBoxLayout* baseLayout = new QVBoxLayout();
    scrollArea.setWidgetResizable(true);
    baseLayout->addWidget(&scrollArea);
    self->setLayout(baseLayout);

    static const char* toolTips[] = { _("Start simulation from the beginning"), _("Start simulation from the current state"),
                                      _("Pause simulation"), _("Stop simulation"), _("Seek backward"), _("Seek forward"),
                                      _("Start playback"), _("Resume playback") };
    QGridLayout* gbox = new QGridLayout;
    MainWindow* mw = MainWindow::instance();
    for(int i = 0; i < NUM_BUTTONS; ++i) {
        ButtonInfo info = buttonInfo[i];
        buttons[i] = new PushButton;
        PushButton* button = buttons[i];
        button->setIconSize(mw->iconSize());
        QIcon icon;
        if(i == BACKWARD) {
            icon = QIcon(mw->style()->standardIcon(QStyle::SP_MediaSeekBackward));
        } else if(i == FORWARD) {
            icon = QIcon(mw->style()->standardIcon(QStyle::SP_MediaSeekForward));
        } else {
            icon = QIcon(info.icon);
        }
        button->setIcon(icon);
        button->setToolTip(toolTips[i]);
        button->setEnabled(info.enabled);
        button->sigClicked().connect([&, i](){ onButtonClicked(i); });
        gbox->addWidget(button, info.row, info.column, info.rowSpan, info.columnSpan);
    }
    buttons[PAUSE]->setCheckable(true);

    static const char* labels[] = { _("Seek time"), _("Time") };
    for(int i = 0; i < NUM_DSPINS; ++i) {
        dspins[i] = new DoubleSpinBox;
        dspins[i]->setDecimals(3);
        dspins[i]->setRange(0.0, 9999.999);
        if(i == SEEKTIME) {
            dspins[i]->setValue(1.0);
            gbox->addWidget(new QLabel(labels[i]), 2, 0, 1, 1);
            gbox->addWidget(dspins[i], 2, 1, 1, 1);
        } else if(i == TIME) {
            dspins[i]->setValue(0.0);
            gbox->addWidget(new QLabel(labels[i]), 1, 0, 1, 1);
            gbox->addWidget(dspins[i], 1, 3, 1, 1);
        }
    }

    timeSlider = new Slider(Qt::Horizontal);
    gbox->addWidget(timeSlider, 1, 1, 1, 2);
    topVBox->addLayout(gbox);

    sb->sigSimulationAboutToStart().connect(
                [&](SimulatorItem* simulatorItem){ onSimulationAboutToStart(simulatorItem); });
    tb->sigTimeChanged().connect([&](double time){ onTimeChanged(time); return true; });
    tb->sigPlaybackStarted().connect([&](double time){ onPlaybackStarted(time); });
    tb->sigPlaybackStopped().connect(
                [&](double time, bool isStoppedManually){ onPlaybackStopped(time , isStoppedManually); });
    dspins[TIME]->sigValueChanged().connect([&](double value){ onValueChanged(value); });
    timeSlider->sigValueChanged().connect([&](int value){ onValueChanged(value); });
}


SimpleSimulationView::~SimpleSimulationView()
{
    delete impl;
}


void SimpleSimulationView::initializeClass(ExtensionManager* ext)
{
    ext->viewManager().registerClass<SimpleSimulationView>(
                N_("SimpleSimulationView"), N_("SimpleSimulation"), ViewManager::SINGLE_OPTIONAL);
}


SimpleSimulationView* SimpleSimulationView::instance()
{
    static SimpleSimulationView* instance_ = ViewManager::findView<SimpleSimulationView>();
    return instance_;
}


void SimpleSimulationViewImpl::onSimulationAboutToStart(SimulatorItem* simulatorItem)
{
    this->simulatorItem = simulatorItem;
    updateButtonStates(START);
}


void SimpleSimulationViewImpl::onButtonClicked(const int& id)
{
    double timestep = 0.0;

    if(id < BACKWARD) {
        if(id == START) {
            sb->startSimulation(true);
        } else if(id == RESTART) {
            sb->startSimulation(false);
        } else if(id == PAUSE) {
            if(simulatorItem) {
                if(buttons[PAUSE]->isChecked()) {
                    simulatorItem->pauseSimulation();
                } else {
                    simulatorItem->restartSimulation();
                }
            }
        } else if(id == STOP) {
            if(simulatorItem) {
                simulatorItem->stopSimulation(true);
                buttons[PAUSE]->setChecked(false);
            }
        }
        updateButtonStates(id);
    } else if(id == PLAY) {
        tb->stopPlayback(true);
        tb->startPlayback(minTime);
    } else if(id == RESUME) {
        if(tb->isDoingPlayback()) {
            tb->stopPlayback(true);
        } else {
            tb->stopPlayback(true);
            tb->startPlayback();
        }
    } else {
        if(id == BACKWARD) {
            timestep = dspins[SEEKTIME]->value() * -1.0;
        } else if(id == FORWARD) {
            timestep = dspins[SEEKTIME]->value();
        }

        buttons[START]->setEnabled(false);
        double time = tb->time() + timestep;
        onValueChanged(time);
    }
}


void SimpleSimulationViewImpl::onValueChanged(const double& value)
{
    if(tb->isDoingPlayback()) {
        tb->stopPlayback(true);
    }
    tb->setTime(value);
}


void SimpleSimulationViewImpl::onValueChanged(const int& value)
{
    if(tb->isDoingPlayback()) {
        tb->stopPlayback(true);
    }
    tb->setTime(value / pow(10.0, decimals));
}


void SimpleSimulationViewImpl::onTimeChanged(const double& time)
{
    const double timeStep = 1.0 / tb->frameRate();
    decimals = static_cast<int>(ceil(log10(tb->frameRate())));
    const double r = pow(10.0, decimals);
    minTime = tb->minTime();
    maxTime = tb->maxTime();
    dspins[TIME]->blockSignals(true);
    dspins[TIME]->setRange(minTime, maxTime);
    dspins[TIME]->setDecimals(decimals);
    dspins[TIME]->setSingleStep(timeStep);
    dspins[TIME]->setValue(time);
    dspins[TIME]->blockSignals(false);

    timeSlider->blockSignals(true);
    timeSlider->setRange((int)nearbyint(minTime * r), (int)nearbyint(maxTime * r));
    timeSlider->setSingleStep(timeStep * r);
    timeSlider->setValue((int)nearbyint(time * pow(10.0, decimals)));
    timeSlider->blockSignals(false);
}


void SimpleSimulationViewImpl::onPlaybackStarted(const double& time)
{
    const static QString tip(_("Stop animation"));
    buttons[RESUME]->setIcon(stopIcon);
    buttons[RESUME]->setToolTip(tip);
}


void SimpleSimulationViewImpl::onPlaybackStopped(const double& time, const bool& isStoppedManually)
{
    const static QString tip(_("Resume animation"));
    buttons[RESUME]->setIcon(resumeIcon);
    buttons[RESUME]->setToolTip(tip);
}


void SimpleSimulationViewImpl::updateButtonStates(const int& state)
{
    ButtonState buttonState = buttonStates[state];
    buttons[START]->setEnabled(buttonState.start);
    buttons[RESTART]->setEnabled(buttonState.restart);
    buttons[PAUSE]->setEnabled(buttonState.pause);
    buttons[STOP]->setEnabled(buttonState.stop);
}
