/**
   @author Kenta Suzuki
*/

#include "OnScreenJoystickWidget.h"
#include <cnoid/Button>
#include <cnoid/Joystick>
#include <cnoid/JoystickCapture>
#include <QBoxLayout>
#include <mutex>
#include <vector>
#include "AxisWidget.h"
#include "gettext.h"

using namespace std;
using namespace cnoid;

namespace {

struct AxisInfo {
    int index;
    int id;
};

AxisInfo axisInfo[] = {
    { 0, 0 },
    { 0, 1 },
    { 1, 0 },
    { 1, 1 },
    { 2, 0 },
    { 2, 1 },
    { 3, 1 },
    { 4, 1 },
};

}

namespace cnoid {

class OnScreenJoystickWidget::Impl
{
public:
    OnScreenJoystickWidget* self;

    Impl(OnScreenJoystickWidget* self);

    enum {
        L_STICK,
        R_STICK,
        DIRECTIONAL_PAD,
        L_TRIGGER_AXIS,
        R_TRIGGER_AXIS,
        NumAxes
    };

    std::mutex mutex;
    vector<double> axisPositions;
    vector<bool> buttonStates;
    AxisWidget* axes[NumAxes];
    ToolButton* buttons[Joystick::NUM_STD_BUTTONS];
    JoystickCapture joystick;

    Signal<void(int id, double position)> sigAxis_;
    Signal<void(int id, bool isPressed)> sigButton_;

    void onAxis(const int& id, const double& position);
    void onButton(const int& id, bool isPressed);
    void onAxis(const int& index, const double& h_position, const double& v_position);
};

}


OnScreenJoystickWidget::OnScreenJoystickWidget(QWidget* parent)
    : QWidget(parent)
{
    impl = new Impl(this);
}


OnScreenJoystickWidget::Impl::Impl(OnScreenJoystickWidget* self)
    : self(self)
{
    axisPositions.resize(Joystick::NUM_STD_AXES, 0.0);
    buttonStates.resize(Joystick::NUM_STD_BUTTONS, false);

    auto topLayout = new QHBoxLayout;
    for(int i = 0; i < NumAxes; ++i) {
        axes[i] = new AxisWidget;
        AxisWidget* axis = axes[i];
        axis->sigAxis().connect(
                    [this, i](double h_position, double v_position){ onAxis(i, h_position, v_position); });
        topLayout->addWidget(axis);
    }

    auto bottomLayout = new QHBoxLayout;
    for(int i = 0; i < Joystick::NUM_STD_BUTTONS; ++i) {
        buttons[i] = new ToolButton(to_string(i).c_str());
        ToolButton* button = buttons[i];
        button->setFixedWidth(30);
        button->sigPressed().connect([this, i](){ onButton(i, true); });
        button->sigReleased().connect([this, i](){ onButton(i, false); });
        bottomLayout->addWidget(button);
    }

    auto vbox = new QVBoxLayout;
    vbox->addLayout(topLayout);
    vbox->addLayout(bottomLayout);

    auto hbox = new QHBoxLayout;
    hbox->addStretch();
    hbox->addLayout(vbox);
    hbox->addStretch();

    auto mainLayout = new QVBoxLayout;
    mainLayout->addStretch();
    mainLayout->addLayout(hbox);
    mainLayout->addStretch();
    self->setLayout(mainLayout);

    joystick.setDevice("/dev/input/js0");
    joystick.sigAxis().connect([&](int id, double position){ onAxis(id, position); });
    joystick.sigButton().connect([&](int id, bool isPressed){ onButton(id, isPressed); });
}


OnScreenJoystickWidget::~OnScreenJoystickWidget()
{
    delete impl;
}


SignalProxy<void(int id, double position)> OnScreenJoystickWidget::sigAxis()
{
    return impl->sigAxis_;
}


SignalProxy<void(int id, bool isPressed)> OnScreenJoystickWidget::sigButton()
{
    return impl->sigButton_;
}


void OnScreenJoystickWidget::Impl::onAxis(const int& id, const double& position)
{
    AxisInfo& info = axisInfo[id];
    axes[info.index]->setValue(info.id, position);
    axisPositions[id] = position;
}


void OnScreenJoystickWidget::Impl::onButton(const int& id, bool isPressed)
{
    std::lock_guard<std::mutex> lock(mutex);
    buttonStates[id] = isPressed;
    sigButton_(id, isPressed);
    QPalette palette;
    if(isPressed) {
        palette.setColor(QPalette::Button, QColor(Qt::red));
    }
    buttons[id]->setPalette(palette);
}


void OnScreenJoystickWidget::Impl::onAxis(const int& index, const double& h_position, const double& v_position)
{
    std::lock_guard<std::mutex> lock(mutex);
    if(index == L_STICK) {
        axisPositions[Joystick::L_STICK_H_AXIS] = h_position;
        axisPositions[Joystick::L_STICK_V_AXIS] = v_position;
        sigAxis_(Joystick::L_STICK_H_AXIS, h_position);
        sigAxis_(Joystick::L_STICK_V_AXIS, v_position);
    } else if(index == R_STICK) {
        axisPositions[Joystick::R_STICK_H_AXIS] = h_position;
        axisPositions[Joystick::R_STICK_V_AXIS] = v_position;
        sigAxis_(Joystick::R_STICK_H_AXIS, h_position);
        sigAxis_(Joystick::R_STICK_V_AXIS, v_position);
    } else if(index == DIRECTIONAL_PAD) {
        axisPositions[Joystick::DIRECTIONAL_PAD_H_AXIS] = h_position;
        axisPositions[Joystick::DIRECTIONAL_PAD_V_AXIS] = v_position;
        sigAxis_(Joystick::DIRECTIONAL_PAD_H_AXIS, h_position);
        sigAxis_(Joystick::DIRECTIONAL_PAD_V_AXIS, v_position);
    } else if(index == L_TRIGGER_AXIS) {
        axisPositions[Joystick::L_TRIGGER_AXIS] = v_position;
        sigAxis_(Joystick::L_TRIGGER_AXIS, v_position);
    } else if(index == R_TRIGGER_AXIS) {
        axisPositions[Joystick::R_TRIGGER_AXIS] = v_position;
        sigAxis_(Joystick::R_TRIGGER_AXIS, v_position);
    }
}