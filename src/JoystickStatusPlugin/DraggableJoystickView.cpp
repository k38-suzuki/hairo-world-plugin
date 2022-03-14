/**
   \file
   \author Kenta Suzuki
*/

#include "DraggableJoystickView.h"
#include <cnoid/Button>
#include <cnoid/ExtJoystick>
#include <cnoid/Joystick>
#include <cnoid/JoystickCapture>
#include <cnoid/ViewManager>
#include <mutex>
#include <vector>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include "AxisWidget.h"
#include "gettext.h"

using namespace cnoid;
using namespace std;

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

class DraggableJoystickViewImpl : public ExtJoystick
{
public:
    DraggableJoystickViewImpl(DraggableJoystickView* self);
    DraggableJoystickView* self;

    enum AxisID {
        L_STICK,
        R_STICK,
        DIRECTIONAL_PAD,
        L_TRIGGER_AXIS,
        R_TRIGGER_AXIS,
        NUM_AXES
    };

    std::mutex mutex;
    vector<double> axisPositions;
    vector<bool> buttonStates;
    AxisWidget* axes[NUM_AXES];
    ToolButton* buttons[Joystick::NUM_STD_BUTTONS];
    JoystickCapture joystick;

    Signal<void(int id, double position)> sigAxis_;
    Signal<void(int id, bool isPressed)> sigButton_;

    void onAxis(const int& id, const double& position);
    void onButton(const int& id, const bool& isPressed);
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


DraggableJoystickView::DraggableJoystickView()
{
    impl = new DraggableJoystickViewImpl(this);
}


DraggableJoystickViewImpl::DraggableJoystickViewImpl(DraggableJoystickView* self)
    : self(self)
{
    self->setDefaultLayoutArea(View::BottomCenterArea);

    axisPositions.resize(Joystick::NUM_STD_AXES, 0.0);
    buttonStates.resize(Joystick::NUM_STD_BUTTONS, false);

    QHBoxLayout* hbox0 = new QHBoxLayout;
    for(int i = 0; i < NUM_AXES; ++i) {
        axes[i] = new AxisWidget;
        AxisWidget* axis = axes[i];
        axis->sigAxis().connect(
                    [=](double h_position, double v_position){ onAxis(i, h_position, v_position); });
        hbox0->addWidget(axis);
    }

    QHBoxLayout* hbox1 = new QHBoxLayout;
    for(int i = 0; i < Joystick::NUM_STD_BUTTONS; ++i) {
        buttons[i] = new ToolButton(to_string(i).c_str());
        ToolButton* button = buttons[i];
        button->setFixedWidth(30);
        button->sigPressed().connect([=](){ onButtonPressed(i); });
        button->sigReleased().connect([=](){ onButtonReleased(i); });
        hbox1->addWidget(button);
    }

    QVBoxLayout* vbox = new QVBoxLayout;
    vbox->addLayout(hbox0);
    vbox->addLayout(hbox1);
    self->setLayout(vbox);

    joystick.setDevice("/dev/input/js0");
    joystick.sigAxis().connect([&](int id, double position){ onAxis(id, position); });
    joystick.sigButton().connect([&](int id, bool isPressed){ onButton(id, isPressed); });

    ExtJoystick::registerJoystick("DraggableJoystickView", this);
}


DraggableJoystickView::~DraggableJoystickView()
{
    delete impl;
}


void DraggableJoystickView::initializeClass(ExtensionManager* ext)
{
    ext->viewManager().registerClass<DraggableJoystickView>(
                N_("DraggableJoystickView"), N_("Draggable Joystick"), ViewManager::SINGLE_OPTIONAL);
}


void DraggableJoystickViewImpl::onAxis(const int& id, const double& position)
{
    AxisInfo info = axisInfo[id];
    axes[info.index]->setValue(info.id, position);
    axisPositions[id] = position;
}


void DraggableJoystickViewImpl::onButton(const int& id, const bool& isPressed)
{
    if(isPressed) {
        onButtonPressed(id);
    } else {
        onButtonReleased(id);
    }
}


void DraggableJoystickViewImpl::onAxis(const int& index, const double& h_position, const double& v_position)
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


void DraggableJoystickViewImpl::onButtonPressed(const int& index)
{
    std::lock_guard<std::mutex> lock(mutex);
    buttonStates[index] = true;
    sigButton_(index, true);
    QPalette palette;
    palette.setColor(QPalette::Button, QColor(Qt::red));
    buttons[index]->setPalette(palette);
}


void DraggableJoystickViewImpl::onButtonReleased(const int& index)
{
    std::lock_guard<std::mutex> lock(mutex);
    buttonStates[index] = false;
    sigButton_(index, false);
    QPalette palette;
    buttons[index]->setPalette(palette);
}


int DraggableJoystickViewImpl::numAxes() const
{
    return axisPositions.size();
}


int DraggableJoystickViewImpl::numButtons() const
{
    return buttonStates.size();
}


bool DraggableJoystickViewImpl::readCurrentState()
{
    return true;
}


double DraggableJoystickViewImpl::getPosition(int axis) const
{
    if(axis >= 0 && axis < (int)axisPositions.size()) {
        return axisPositions[axis];
    }
    return 0.0;
}


bool DraggableJoystickViewImpl::getButtonState(int button) const
{
    if(button >= 0 && button < (int)buttonStates.size()) {
        return buttonStates[button];
    }
    return false;
}


bool DraggableJoystickViewImpl::isActive() const
{
    for(size_t i = 0; i < axisPositions.size(); ++i) {
        if(axisPositions[i] != 0.0) {
            return true;
        }
    }
    for(size_t i = 0; i < buttonStates.size(); ++i) {
        if(buttonStates[i]) {
            return true;
        }
    }
    return false;
}


SignalProxy<void(int id, double position)> DraggableJoystickViewImpl::sigAxis()
{
    return sigAxis_;
}


SignalProxy<void(int id, bool isPressed)> DraggableJoystickViewImpl::sigButton()
{
    return sigButton_;
}


bool DraggableJoystickView::storeState(Archive& archive)
{
    return true;
}


bool DraggableJoystickView::restoreState(const Archive& archive)
{
    return true;
}
