/**
   @author Kenta Suzuki
*/

#include "VirtualJoystickWidget.h"
#include <cnoid/Buttons>
#include <cnoid/ExtJoystick>
#include <cnoid/JoystickCapture>
#include <QBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QKeyEvent>
#include <QStackedLayout>
#include <thread>
#include <mutex>
#include "OnScreenJoystickWidget.h"
#include "gettext.h"

using namespace std;
using namespace cnoid;

namespace {

enum {
    ARROW_UP, ARROW_DOWN, ARROW_LEFT, ARROW_RIGHT,
    L_AXIS_UP, L_AXIS_DOWN, L_AXIS_LEFT, L_AXIS_RIGHT,
    R_AXIS_UP, R_AXIS_DOWN, R_AXIS_LEFT, R_AXIS_RIGHT,
    L_AXIS_TRIGGER, R_AXIS_TRIGGER,
    BUTTON_A, BUTTON_B, BUTTON_X, BUTTON_Y,
    BUTTON_L, BUTTON_R, BUTTON_SELECT, BUTTON_START,
    BUTTON_L_STICK, BUTTON_R_STICK, BUTTON_LOGO,
    NUM_JOYSTICK_ELEMENTS
};

struct ButtonInfo {
    const char* label;
    int row;
    int column;
    bool isAxis;
    double activeValue;
    int id;
    int key;
};

ButtonInfo buttonInfo[] = {

    { "^", 0,  1,  true, -1.0, 5,    Qt::Key_Up },
    { "v", 2,  1,  true,  1.0, 5,  Qt::Key_Down },
    { "<", 1,  0,  true, -1.0, 4,  Qt::Key_Left },
    { ">", 1,  2,  true,  1.0, 4, Qt::Key_Right },

    { "E", 3,  3,  true, -1.0, 1,     Qt::Key_E },
    { "D", 5,  3,  true,  1.0, 1,     Qt::Key_D },
    { "S", 4,  2,  true, -1.0, 0,     Qt::Key_S },
    { "F", 4,  4,  true,  1.0, 0,     Qt::Key_F },

    { "I", 3,  7,  true, -1.0, 3,     Qt::Key_I },
    { "K", 5,  7,  true,  1.0, 3,     Qt::Key_K },
    { "J", 4,  6,  true, -1.0, 2,     Qt::Key_J },
    { "L", 4,  8,  true,  1.0, 2,     Qt::Key_L },

    { "Q", 3,  1,  true, 1.0,  6,     Qt::Key_Q },
    { "P", 3,  9,  true, 1.0,  7,     Qt::Key_P },

    { "A", 2,  9, false, 1.0,  0,     Qt::Key_A },
    { "B", 1, 10, false, 1.0,  1,     Qt::Key_B },
    { "X", 1,  8, false, 1.0,  2,     Qt::Key_X },
    { "Y", 0,  9, false, 1.0,  3,     Qt::Key_Y },

    { "W", 3,  2, false, 1.0,  4,     Qt::Key_W },
    { "O", 3,  8, false, 1.0,  5,     Qt::Key_O },
    { "R", 3,  4, false, 1.0,  6,     Qt::Key_R },
    { "U", 3,  6, false, 1.0,  7,     Qt::Key_U },

    { "V", 5,  4, false, 1.0,  8,     Qt::Key_V },
    { "N", 5,  6, false, 1.0,  9,     Qt::Key_N },
    { " ", 5,  5, false, 1.0, 10, Qt::Key_Space }

};

struct VirtualAxisInfo {
    int realId;
    bool isPositive;
    int virtualId;
};

VirtualAxisInfo virtualAxisInfo[] = {
    { 0, false,  6 },
    { 0,  true,  7 },
    { 1, false,  4 },
    { 1,  true,  5 },

    { 2, false, 10 },
    { 2,  true, 11 },
    { 3, false,  8 },
    { 3,  true,  9 },

    { 4, false,  2 },
    { 4,  true,  3 },
    { 5, false,  0 },
    { 5,  true,  1 },

    { 6,  true, 12 },
    { 7,  true, 13 }
};

struct VirtualButtonInfo {
    int realId;
    int virtualId;
};

VirtualButtonInfo virtualButtonInfo[] = {
    {  0, 14 },
    {  1, 15 },
    {  2, 16 },
    {  3, 17 },

    {  4, 18 },
    {  5, 19 },
    {  6, 20 },
    {  7, 21 },

    {  8, 22 },
    {  9, 23 },
    { 10, 24 }
};

}

namespace cnoid {

class VirtualJoystickWidget::Impl : public ExtJoystick
{
public:
    VirtualJoystickWidget* self;

    Impl(VirtualJoystickWidget* self);

    QGridLayout grid;
    QStackedLayout* stackedLayout;
    ToolButton buttons[NUM_JOYSTICK_ELEMENTS];
    typedef std::map<int, int> KeyToButtonMap;
    KeyToButtonMap keyToButtonMap;
    vector<double> keyValues;
    Signal<void(int id, bool isPressed)> sigButton_;
    Signal<void(int id, double position)> sigAxis_;
    std::mutex mutex;
    vector<double> axisPositions;
    vector<bool> buttonStates;
    JoystickCapture joystick;

    bool onKeyStateChanged(int key, bool on);
    void onButtonPressed(int index);
    void onButtonReleased(int index);
    void onButtonClicked(int id, bool isPressed);
    void onAxis(int id, double position);
    void onButton(int id, bool isPressed);
    void onVirtualAxis(int id, double position);
    void onVirtualButton(int id, bool isPressed);

    virtual int numAxes() const;
    virtual int numButtons() const;
    virtual bool readCurrentState();
    virtual double getPosition(int axis) const;
    virtual bool getButtonState(int button) const;
    virtual bool isActive() const;
    virtual SignalProxy<void(int id, double position)> sigAxis();
    virtual SignalProxy<void(int id, bool isPressed)> sigButton();
};

}


VirtualJoystickWidget::VirtualJoystickWidget(QWidget* parent)
    : QWidget(parent)
{
    impl = new Impl(this);
}


VirtualJoystickWidget::Impl::Impl(VirtualJoystickWidget* self)
    : self(self),
      keyValues(NUM_JOYSTICK_ELEMENTS, 0.0)
{
    self->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    self->setFocusPolicy(Qt::WheelFocus);

    for(int i = 0; i < NUM_JOYSTICK_ELEMENTS; ++i) {
        ButtonInfo& info = buttonInfo[i];
        ToolButton& button = buttons[i];
        button.setText(info.label);
        grid.addWidget(&buttons[i], info.row, info.column);
        keyToButtonMap[info.key] = i;
        if(info.isAxis) {
            if(info.id >= static_cast<int>(axisPositions.size())) {
                axisPositions.resize(info.id + 1, 0.0);
            }
        } else {
            if(info.id >= static_cast<int>(buttonStates.size())) {
                buttonStates.resize(info.id + 1, false);
            }
        }

        button.sigPressed().connect([this, i](){ onButtonPressed(i); });
        button.sigReleased().connect([this, i](){ onButtonReleased(i); });
    }

    joystick.setDevice("/dev/input/js0");
    joystick.sigAxis().connect([&](int id, double position){ onAxis(id, position); });
    joystick.sigButton().connect([&](int id, bool isPressed){ onButton(id, isPressed); });

    auto hbox = new QHBoxLayout;
    hbox->addStretch();
    hbox->addLayout(&grid);
    hbox->addStretch();
    auto vbox = new QVBoxLayout;
    vbox->addStretch();
    vbox->addLayout(hbox);
    vbox->addStretch();

    auto firstPageWidget = new QWidget;
    firstPageWidget->setLayout(vbox);

    auto secondPageWidget = new OnScreenJoystickWidget;
    secondPageWidget->sigAxis().connect(
        [&](int id, double position){ onVirtualAxis(id, position); });
    secondPageWidget->sigButton().connect(
        [&](int id, bool isPressed){ onVirtualButton(id, isPressed); });

    stackedLayout = new QStackedLayout;
    stackedLayout->addWidget(firstPageWidget);
    stackedLayout->addWidget(secondPageWidget);
    self->setLayout(stackedLayout);

    ExtJoystick::registerJoystick("VirtualJoystickView2", this);
}


VirtualJoystickWidget::~VirtualJoystickWidget()
{
    delete impl;
}


void VirtualJoystickWidget::setViewMode(ViewMode mode)
{
    impl->stackedLayout->setCurrentIndex(mode);
}


void VirtualJoystickWidget::keyPressEvent(QKeyEvent* event)
{
    if(!impl->onKeyStateChanged(event->key(), true)) {
        QWidget::keyPressEvent(event);
    }
}


void VirtualJoystickWidget::keyReleaseEvent(QKeyEvent* event)
{
    if(!impl->onKeyStateChanged(event->key(), false)) {
        QWidget::keyPressEvent(event);
    }
}


bool VirtualJoystickWidget::Impl::onKeyStateChanged(int key, bool on)
{
    KeyToButtonMap::iterator p = keyToButtonMap.find(key);
    if(p == keyToButtonMap.end()) {
        return false;
    } else {
        int index = p->second;
        ToolButton& button = buttons[index];
        ButtonInfo& info = buttonInfo[index];
        button.setDown(on);
        {
            std::lock_guard<std::mutex> lock(mutex);
            keyValues[index] = on ? info.activeValue : 0.0;
        }
        onButtonClicked(index, on);
    }
    return true;
}


void VirtualJoystickWidget::Impl::onButtonPressed(int index)
{
    ButtonInfo& info = buttonInfo[index];
    std::lock_guard<std::mutex> lock(mutex);
    keyValues[index] = info.activeValue;
    onButtonClicked(index, true);
}


void VirtualJoystickWidget::Impl::onButtonReleased(int index)
{
    std::lock_guard<std::mutex> lock(mutex);
    keyValues[index] = 0.0;
    onButtonClicked(index, false);
}


void VirtualJoystickWidget::Impl::onButtonClicked(int id, bool isPressed)
{
    ToolButton& button = buttons[id];
    ButtonInfo& info = buttonInfo[id];
    QPalette palette;
    if(isPressed) {
        palette.setColor(QPalette::Button, QColor(Qt::red));
    }
    button.setPalette(palette);
}


void VirtualJoystickWidget::Impl::onAxis(int id, double position)
{
    for(int i = 0; i < NUM_JOYSTICK_ELEMENTS; ++i) {
        ToolButton& button = buttons[i];
        ButtonInfo& info = buttonInfo[i];
        if(info.isAxis) {
            if(info.id == id) {
                QPalette palette;
                if((info.activeValue < 0.0 && position < 0.0)
                        || (info.activeValue > 0.0 && position > 0.0)) {
                    palette.setColor(QPalette::Button, QColor(Qt::red));
                }
                button.setPalette(palette);
            }
        }
    }
}


void VirtualJoystickWidget::Impl::onButton(int id, bool isPressed)
{
    for(int i = 0; i < NUM_JOYSTICK_ELEMENTS; ++i) {
        ButtonInfo& info = buttonInfo[i];
        if(!info.isAxis) {
            if(info.id == id) {
                onButtonClicked(i, isPressed);
            }
        }
    }
}


void VirtualJoystickWidget::Impl::onVirtualAxis(int id, double position)
{
    for(int i = 0; i < 14; ++i) {
        VirtualAxisInfo& info = virtualAxisInfo[i];
        if(info.realId == id) {
            if(info.isPositive && position > 0.0) {
                onButtonPressed(info.virtualId);
            } else if(!info.isPositive && position < 0.0) {
                onButtonPressed(info.virtualId);
            } else {
                onButtonReleased(info.virtualId);
            }
        }
    }
    onAxis(id, position);
}


void VirtualJoystickWidget::Impl::onVirtualButton(int id, bool isPressed)
{
    for(int i = 0; i < 11; ++i) {
        VirtualButtonInfo& info = virtualButtonInfo[i];
        if(info.realId == id) {
            if(isPressed) {
                onButtonPressed(info.virtualId);
            } else {
                onButtonReleased(info.virtualId);
            }
        }
    }
    onButton(id, isPressed);
}


int VirtualJoystickWidget::Impl::numAxes() const
{
    return axisPositions.size();
}


int VirtualJoystickWidget::Impl::numButtons() const
{
    return buttonStates.size();
}


bool VirtualJoystickWidget::Impl::readCurrentState()
{
    std::fill(axisPositions.begin(), axisPositions.end(), 0.0);

    {
        std::lock_guard<std::mutex> lock(mutex);
        for(int i = 0; i < NUM_JOYSTICK_ELEMENTS; ++i) {
            ButtonInfo& info = buttonInfo[i];
            if(info.isAxis) {
                axisPositions[info.id] += keyValues[i];
            } else {
                buttonStates[info.id] = keyValues[i];
            }
        }
    }
    return true;
}


double VirtualJoystickWidget::Impl::getPosition(int axis) const
{
    if(axis >=0 && axis < static_cast<int>(axisPositions.size())) {
        return axisPositions[axis];
    }
    return 0.0;
}


bool VirtualJoystickWidget::Impl::getButtonState(int button) const
{
    if(button >= 0 && button < static_cast<int>(buttonStates.size())) {
        return buttonStates[button];
    }
    return false;
}


bool VirtualJoystickWidget::Impl::isActive() const
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


SignalProxy<void(int id, bool isPressed)> VirtualJoystickWidget::Impl::sigButton()
{
    return sigButton_;
}


SignalProxy<void(int id, double position)> VirtualJoystickWidget::Impl::sigAxis()
{
    return sigAxis_;
}