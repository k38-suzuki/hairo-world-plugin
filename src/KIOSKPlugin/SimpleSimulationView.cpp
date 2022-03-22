/**
   \file
   \author Kenta Suzuki
*/

#include "SimpleSimulationView.h"
#include <cnoid/Button>
#include <cnoid/MainWindow>
#include <cnoid/SimulationBar>
#include <cnoid/SimulatorItem>
#include <cnoid/SpinBox>
#include <cnoid/TimeBar>
#include <cnoid/ViewManager>
#include <cnoid/Widget>
#include <QGridLayout>
#include <QLabel>
#include <QScrollArea>
#include <QStyle>
#include <QVBoxLayout>
#include "SimpleTimeWidget.h"
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
    { 1, 2, 1, 1,  true,                                   "" },
    { 1, 3, 1, 1,  true,                                   "" },
    { 2, 0, 1, 1,  true,               ":/Base/icon/play.svg" },
    { 2, 1, 1, 1,  true,             ":/Base/icon/resume.svg" }
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

    QScrollArea scrollArea;

    PushButton* buttons[NUM_BUTTONS];
    DoubleSpinBox* seekDSpin;
    SimulatorItem* simulatorItem;
    SimulationBar* sb;
    TimeBar* tb;

    QIcon resumeIcon;
    QIcon stopIcon;

    void onSimulationAboutToStart(SimulatorItem* simulatorItem);
    void onButtonClicked(const int& id);
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
    QVBoxLayout* baseLayout = new QVBoxLayout;
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

    seekDSpin = new DoubleSpinBox;
    seekDSpin->setDecimals(3);
    seekDSpin->setRange(0.0, 9999.999);
    seekDSpin->setValue(1.0);
    gbox->addWidget(new QLabel(_("Seek time")), 1, 0, 1, 1);
    gbox->addWidget(seekDSpin, 1, 1, 1, 1);

    SimpleTimeWidget* timeWidget = new SimpleTimeWidget;
    topVBox->addWidget(timeWidget);
    topVBox->addLayout(gbox);
    topVBox->addStretch();

    simulatorItem = nullptr;

    sb->sigSimulationAboutToStart().connect(
                [&](SimulatorItem* simulatorItem){ onSimulationAboutToStart(simulatorItem); });
    tb->sigPlaybackStarted().connect([&](double time){ onPlaybackStarted(time); });
    tb->sigPlaybackStopped().connect(
                [&](double time, bool isStoppedManually){ onPlaybackStopped(time , isStoppedManually); });
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
        tb->startPlayback(tb->minTime());
    } else if(id == RESUME) {
        if(tb->isDoingPlayback()) {
            tb->stopPlayback(true);
        } else {
            tb->stopPlayback(true);
            tb->startPlayback();
        }
    } else {
        if(id == BACKWARD) {
            timestep = seekDSpin->value() * -1.0;
        } else if(id == FORWARD) {
            timestep = seekDSpin->value();
        }

        buttons[START]->setEnabled(false);
        double time = tb->time() + timestep;
        if(tb->isDoingPlayback()) {
            tb->stopPlayback(true);
        }
        tb->setTime(time);
    }
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
