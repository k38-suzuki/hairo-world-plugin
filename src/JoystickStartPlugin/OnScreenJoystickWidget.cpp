/**
   @author Kenta Suzuki
*/

#include "OnScreenJoystickWidget.h"
#include <cnoid/Button>
#include <cnoid/ExtJoystick>
#include <cnoid/Joystick>
#include <cnoid/JoystickCapture>
#include <mutex>
#include <vector>
#include <QBoxLayout>
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

class OnScreenJoystickWidget::Impl : public ExtJoystick
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
    void onButtonPressed(const int& index);
    void onButtonReleased(const int& index);

    virtual int numAxes() const override;
    virtual int numButtons() const override;
    virtual bool readCurrentState() override;
    virtual double getPosition(int axis) const override;
    virtual bool getButtonState(int button) const override;
    virtual bool isActive() const override;
    virtual SignalProxy<void(int id, double position)> sigAxis() override;
    virtual SignalProxy<void(int id, bool isPressed)> sigButton() override;
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

    auto hbox1 = new QHBoxLayout;
    for(int i = 0; i < NumAxes; ++i) {
        axes[i] = new AxisWidget;
        AxisWidget* axis = axes[i];
        axis->sigAxis().connect(
                    [this, i](double h_position, double v_position){ onAxis(i, h_position, v_position); });
        hbox1->addWidget(axis);
    }

    auto hbox2 = new QHBoxLayout;
    for(int i = 0; i < Joystick::NUM_STD_BUTTONS; ++i) {
        buttons[i] = new ToolButton(to_string(i).c_str());
        ToolButton* button = buttons[i];
        button->setFixedWidth(30);
        button->sigPressed().connect([this, i](){ onButtonPressed(i); });
        button->sigReleased().connect([this, i](){ onButtonReleased(i); });
        hbox2->addWidget(button);
    }

    auto vbox1 = new QVBoxLayout;
    vbox1->addLayout(hbox1);
    vbox1->addLayout(hbox2);

    auto hbox = new QHBoxLayout;
    hbox->addStretch();
    hbox->addLayout(vbox1);
    hbox->addStretch();
    auto vbox = new QVBoxLayout;
    vbox->addStretch();
    vbox->addLayout(hbox);
    vbox->addStretch();
    self->setLayout(vbox);

    joystick.setDevice("/dev/input/js0");
    joystick.sigAxis().connect([&](int id, double position){ onAxis(id, position); });
    joystick.sigButton().connect([&](int id, bool isPressed){ onButton(id, isPressed); });

    ExtJoystick::registerJoystick("OnScreenJoystickView", this);
}


OnScreenJoystickWidget::~OnScreenJoystickWidget()
{
    delete impl;
}


void OnScreenJoystickWidget::Impl::onAxis(const int& id, const double& position)
{
    AxisInfo& info = axisInfo[id];
    axes[info.index]->setValue(info.id, position);
    axisPositions[id] = position;
}


void OnScreenJoystickWidget::Impl::onButton(const int& id, bool isPressed)
{
    if(isPressed) {
        onButtonPressed(id);
    } else {
        onButtonReleased(id);
    }
}


void OnScreenJoystickWidget::Impl::onAxis(const int& index, const double& h_position, const double& v_position)
{
    std::lock_guard<std::mutex> lock(mutex);
    if(index == L_STICK) {
        axisPositions[Joystick::L_STICK_H_AXIS] = h_position;
        sigAxis_(index, h_position);
        axisPositions[Joystick::L_STICK_V_AXIS] = v_position;
    } else if(index == R_STICK) {
        axisPositions[Joystick::R_STICK_H_AXIS] = h_position;
        sigAxis_(index, h_position);
        axisPositions[Joystick::R_STICK_V_AXIS] = v_position;
    } else if(index == DIRECTIONAL_PAD) {
        axisPositions[Joystick::DIRECTIONAL_PAD_H_AXIS] = h_position;
        sigAxis_(index, h_position);
        axisPositions[Joystick::DIRECTIONAL_PAD_V_AXIS] = v_position;
    } else if(index == L_TRIGGER_AXIS) {
        axisPositions[Joystick::L_TRIGGER_AXIS] = v_position;
    } else if(index == R_TRIGGER_AXIS) {
        axisPositions[Joystick::R_TRIGGER_AXIS] = v_position;
    }
    sigAxis_(index, v_position);
}


void OnScreenJoystickWidget::Impl::onButtonPressed(const int& index)
{
    std::lock_guard<std::mutex> lock(mutex);
    buttonStates[index] = true;
    sigButton_(index, true);
    QPalette palette;
    palette.setColor(QPalette::Button, QColor(Qt::red));
    buttons[index]->setPalette(palette);
}


void OnScreenJoystickWidget::Impl::onButtonReleased(const int& index)
{
    std::lock_guard<std::mutex> lock(mutex);
    buttonStates[index] = false;
    sigButton_(index, false);
    QPalette palette;
    buttons[index]->setPalette(palette);
}


int OnScreenJoystickWidget::Impl::numAxes() const
{
    return axisPositions.size();
}


int OnScreenJoystickWidget::Impl::numButtons() const
{
    return buttonStates.size();
}


bool OnScreenJoystickWidget::Impl::readCurrentState()
{
    return true;
}


double OnScreenJoystickWidget::Impl::getPosition(int axis) const
{
    if(axis >= 0 && axis < (int)axisPositions.size()) {
        return axisPositions[axis];
    }
    return 0.0;
}


bool OnScreenJoystickWidget::Impl::getButtonState(int button) const
{
    if(button >= 0 && button < (int)buttonStates.size()) {
        return buttonStates[button];
    }
    return false;
}


bool OnScreenJoystickWidget::Impl::isActive() const
{
    for(auto& axisPosition : axisPositions) {
        if(axisPosition != 0.0) {
            return true;
        }
    }
    for(auto buttonState : buttonStates) {
        if(buttonState) {
            return true;
        }
    }
    return false;
}


SignalProxy<void(int id, double position)> OnScreenJoystickWidget::Impl::sigAxis()
{
    return sigAxis_;
}


SignalProxy<void(int id, bool isPressed)> OnScreenJoystickWidget::Impl::sigButton()
{
    return sigButton_;
}
