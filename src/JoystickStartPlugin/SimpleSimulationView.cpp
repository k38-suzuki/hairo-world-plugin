/**
   @author Kenta Suzuki
*/

#include "SimpleSimulationView.h"
#include <cnoid/Button>
#include <cnoid/ButtonGroup>
#include <cnoid/MainWindow>
#include <cnoid/SimulationBar>
#include <cnoid/SimulatorItem>
#include <cnoid/SpinBox>
#include <cnoid/TimeBar>
#include <cnoid/ViewManager>
#include <cnoid/Widget>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QScrollArea>
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
    bool backward;
    bool forward;
};

ButtonState buttonStates[] = {
    { false, false,  true,  true, false, false },
    { false, false,  true,  true, false, false },
    { false, false,  true,  true, false, false },
    {  true,  true, false, false,  true,  true }
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
    { 0, 2, 1, 1,  true,   ":/Body/icon/start-simulation.svg" },
    { 0, 3, 1, 1,  true, ":/Body/icon/restart-simulation.svg" },
    { 0, 4, 1, 1, false,   ":/Body/icon/pause-simulation.svg" },
    { 0, 0, 1, 1, false,    ":/Body/icon/stop-simulation.svg" },
    { 0, 1, 1, 1,  true,                                   "" },
    { 0, 5, 1, 1,  true,                                   "" }
};

}

namespace cnoid {

class SimpleSimulationView::Impl
{
public:
    SimpleSimulationView* self;

    Impl(SimpleSimulationView* self);

    enum ButtonID {
        START, RESTART, PAUSE, STOP,
        BACKWARD, FORWARD, NUM_BUTTONS
    };

    QScrollArea scrollArea;

    ButtonGroup playModeGroup;
    RadioButton simulationRadio;
    RadioButton animationRadio;
    PushButton* buttons[NUM_BUTTONS];
    DoubleSpinBox* seekDSpin;
    SimulatorItem* simulatorItem;
    SimulationBar* sb;
    TimeBar* tb;

    QIcon startIcon;
    QIcon restartIcon;
    QIcon playIcon;
    QIcon resumeIcon;
    QIcon stopIcon;

    void onSimulationAboutToStart(SimulatorItem* simulatorItem);
    void onButtonClicked(const int& id);
    void onSimulationButtonToggled(const bool& checked);
    void onPlaybackStarted(const double& time);
    void onPlaybackStopped(const double& time, const bool& isStoppedManually);
    void updateButtonStates(const int& state);
};

}


SimpleSimulationView::SimpleSimulationView()
{
    impl = new Impl(this);
}


SimpleSimulationView::Impl::Impl(SimpleSimulationView* self)
    : self(self),
      sb(SimulationBar::instance()),
      tb(TimeBar::instance()),
      startIcon(QIcon(":/Body/icon/start-simulation.svg")),
      restartIcon(QIcon(":/Body/icon/restart-simulation.svg")),
      playIcon(QIcon(":/Base/icon/play.svg")),
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

    QHBoxLayout* hbox0 = new QHBoxLayout;
    hbox0->addWidget(new QLabel(_("Play mode")));
    simulationRadio.setText(_("Simulation"));
    simulationRadio.setChecked(true);
    hbox0->addWidget(&simulationRadio);
    playModeGroup.addButton(&simulationRadio, 0);

    animationRadio.setText(_("Animation"));
    animationRadio.setChecked(false);
    hbox0->addWidget(&animationRadio);
    playModeGroup.addButton(&animationRadio, 1);
    hbox0->addStretch();

    static const char* toolTips[] = {
        _("Start simulation from the beginning"), _("Start simulation from the current state"),
        _("Pause simulation"), _("Stop simulation"), _("Seek backward"), _("Seek forward")
    };
    QGridLayout* gbox = new QGridLayout;
    for(int i = 0; i < NUM_BUTTONS; ++i) {
        ButtonInfo info = buttonInfo[i];
        buttons[i] = new PushButton;
        PushButton* button = buttons[i];
        button->setIconSize(MainWindow::instance()->iconSize());
        QIcon icon;
        if(i == BACKWARD) {
            icon = QIcon::fromTheme("media-seek-backward");
        } else if(i == FORWARD) {
            icon = QIcon::fromTheme("media-seek-forward");
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

    QHBoxLayout* hbox1 = new QHBoxLayout;
    seekDSpin = new DoubleSpinBox;
    seekDSpin->setDecimals(3);
    seekDSpin->setRange(0.0, 9999.999);
    seekDSpin->setValue(1.0);
    hbox1->addStretch();
    hbox1->addWidget(new QLabel(_("Seek time")));
    hbox1->addWidget(seekDSpin);

    SimpleTimeWidget* timeWidget = new SimpleTimeWidget;

    topVBox->addLayout(hbox0);
    topVBox->addWidget(timeWidget);
    topVBox->addLayout(gbox);
    topVBox->addLayout(hbox1);
    topVBox->addStretch();

    simulatorItem = nullptr;

    simulationRadio.sigToggled().connect(
                [&](bool checked){ onSimulationButtonToggled(checked); });

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


void SimpleSimulationView::Impl::onSimulationAboutToStart(SimulatorItem* simulatorItem)
{
    this->simulatorItem = simulatorItem;
    updateButtonStates(START);
}


void SimpleSimulationView::Impl::onButtonClicked(const int& id)
{
    double timestep = 0.0;

    if(id < BACKWARD) {
        if(id == START) {
            if(simulationRadio.isChecked()) {
                sb->startSimulation(true);
            } else {
                tb->stopPlayback(true);
                tb->startPlayback(tb->minTime());
            }
        } else if(id == RESTART) {
            if(simulationRadio.isChecked()) {
                sb->startSimulation(false);
            } else {
                if(tb->isDoingPlayback()) {
                    tb->stopPlayback(true);
                } else {
                    tb->stopPlayback(true);
                    tb->startPlayback();
                }
            }
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
                simulationRadio.setEnabled(true);
                animationRadio.setEnabled(true);
            }
        }
        updateButtonStates(id);
    } else {
        if(id == BACKWARD) {
            timestep = seekDSpin->value() * -1.0;
        } else if(id == FORWARD) {
            timestep = seekDSpin->value();
        }

        double time = tb->time() + timestep;
        if(tb->isDoingPlayback()) {
            tb->stopPlayback(true);
        }
        tb->setTime(time);
    }
}


void SimpleSimulationView::Impl::onSimulationButtonToggled(const bool& checked)
{
    const static QStringList tips = {
        _("Start simulation from the beginning"), _("Start simulation from the current state"),
        _("Start playback"), _("Resume playback")
    };

    if(checked) {
        buttons[START]->setIcon(startIcon);
        buttons[START]->setToolTip(tips[0]);
        buttons[RESTART]->setIcon(restartIcon);
        buttons[RESTART]->setToolTip(tips[1]);
    } else {
        buttons[START]->setIcon(playIcon);
        buttons[START]->setToolTip(tips[2]);
        buttons[RESTART]->setIcon(resumeIcon);
        buttons[RESTART]->setToolTip(tips[3]);
    }
}


void SimpleSimulationView::Impl::onPlaybackStarted(const double& time)
{
    if(simulationRadio.isChecked()) {

    } else {
        const static QString tip(_("Stop animation"));
        buttons[RESTART]->setIcon(stopIcon);
        buttons[RESTART]->setToolTip(tip);
    }
    simulationRadio.setEnabled(false);
    animationRadio.setEnabled(false);
}


void SimpleSimulationView::Impl::onPlaybackStopped(const double& time, const bool& isStoppedManually)
{
    if(simulationRadio.isChecked()) {

    } else {
        const static QString tip(_("Resume animation"));
        buttons[RESTART]->setIcon(resumeIcon);
        buttons[RESTART]->setToolTip(tip);
    }
    if(!buttons[PAUSE]->isChecked()) {
        simulationRadio.setEnabled(true);
        animationRadio.setEnabled(true);
    }
}


void SimpleSimulationView::Impl::updateButtonStates(const int& state)
{
    if(simulationRadio.isChecked()) {
        ButtonState buttonState = buttonStates[state];
        buttons[START]->setEnabled(buttonState.start);
        buttons[RESTART]->setEnabled(buttonState.restart);
        buttons[PAUSE]->setEnabled(buttonState.pause);
        buttons[STOP]->setEnabled(buttonState.stop);
        buttons[BACKWARD]->setEnabled(buttonState.backward);
        buttons[FORWARD]->setEnabled(buttonState.forward);
    }
}
