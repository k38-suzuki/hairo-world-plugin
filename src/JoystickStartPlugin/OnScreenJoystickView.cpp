/**
   \file
   \author  Kenta Suzuki
*/

#include "OnScreenJoystickView.h"
#include <cnoid/ActionGroup>
#include <cnoid/Archive>
#include <cnoid/Buttons>
#include <cnoid/ExtJoystick>
#include <cnoid/JoystickCapture>
#include <cnoid/MenuManager>
#include <cnoid/ViewManager>
#include <cnoid/Widget>
#include <QBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QKeyEvent>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <thread>
#include <mutex>
#include <cmath>
#include "OnScreenJoystickWidget.h"
#include "gettext.h"

using namespace cnoid;
using namespace std;

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
    
}

namespace cnoid {

class OnScreenJoystickViewImpl : public ExtJoystick
{
public:
    OnScreenJoystickViewImpl(OnScreenJoystickView* self);
    OnScreenJoystickView* self;

    QGridLayout grid;
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
    bool isOnScreenJoystickEnabled;
    QStackedWidget* topWidget;

    bool onKeyStateChanged(int key, bool on);
    void onButtonPressed(int index);
    void onButtonReleased(int index);
    void onButtonClicked( const int& id, const bool& isPressed);
    void onAxis(const int& id, const double& position);
    void onButton(const int& id, const bool& isPressed);
    
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


void OnScreenJoystickView::initializeClass(ExtensionManager* ext)
{
    ext->viewManager().registerClass<OnScreenJoystickView>(
        "OnScreenJoystickView", N_("Virtual Joystick2"), ViewManager::SINGLE_OPTIONAL);
}


OnScreenJoystickView::OnScreenJoystickView()
{
    impl = new OnScreenJoystickViewImpl(this);
}


OnScreenJoystickViewImpl::OnScreenJoystickViewImpl(OnScreenJoystickView* self)
    : self(self),
      keyValues(NUM_JOYSTICK_ELEMENTS, 0.0)
{
    self->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    self->setDefaultLayoutArea(View::BottomCenterArea);
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

        button.sigPressed().connect([=](){ onButtonPressed(i); });
        button.sigReleased().connect([=](){ onButtonReleased(i); });
    }

    joystick.setDevice("/dev/input/js0");
    joystick.sigAxis().connect([&](int id, double position){ onAxis(id, position); });
    joystick.sigButton().connect([&](int id, bool isPressed){ onButton(id, isPressed); });

    Widget* joystickWidget = new Widget;
    {
        QHBoxLayout* hbox = new QHBoxLayout;
        hbox->addStretch();
        hbox->addLayout(&grid);
        hbox->addStretch();
        QVBoxLayout* vbox = new QVBoxLayout;
        vbox->addStretch();
        vbox->addLayout(hbox);
        vbox->addStretch();
        joystickWidget->setLayout(vbox);   
    }

    topWidget = new QStackedWidget;
    topWidget->addWidget(joystickWidget);
    topWidget->addWidget(new OnScreenJoystickWidget);

    QVBoxLayout* vbox = new QVBoxLayout;
    vbox->addWidget(topWidget);
    self->setLayout(vbox);

    ExtJoystick::registerJoystick("OnScreenJoystickView", this);
}


OnScreenJoystickView::~OnScreenJoystickView()
{
    delete impl;
}


void OnScreenJoystickView::keyPressEvent(QKeyEvent* event)
{
    if(!impl->onKeyStateChanged(event->key(), true)) {
        View::keyPressEvent(event);
    }
}


void OnScreenJoystickView::keyReleaseEvent(QKeyEvent* event)
{
    if(!impl->onKeyStateChanged(event->key(), false)) {
        View::keyPressEvent(event);
    }
}


bool OnScreenJoystickViewImpl::onKeyStateChanged(int key, bool on)
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


void OnScreenJoystickViewImpl::onButtonPressed(int index)
{
    ButtonInfo& info = buttonInfo[index];
    std::lock_guard<std::mutex> lock(mutex);
    keyValues[index] = info.activeValue;
    onButtonClicked(index, true);
}


void OnScreenJoystickViewImpl::onButtonReleased(int index)
{
    std::lock_guard<std::mutex> lock(mutex);
    keyValues[index] = 0.0;
    onButtonClicked(index, false);
}


void OnScreenJoystickViewImpl::onButtonClicked(const int& id, const bool& isPressed)
{
    ToolButton& button = buttons[id];
    ButtonInfo& info = buttonInfo[id];
    QPalette palette;
    if(isPressed) {
        palette.setColor(QPalette::Button, QColor(Qt::red));
    }
    button.setPalette(palette);
}


void OnScreenJoystickViewImpl::onAxis(const int& id, const double& position)
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


void OnScreenJoystickViewImpl::onButton(const int& id, const bool& isPressed)
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


int OnScreenJoystickViewImpl::numAxes() const
{
    return axisPositions.size();
}


int OnScreenJoystickViewImpl::numButtons() const
{
    return buttonStates.size();
}


bool OnScreenJoystickViewImpl::readCurrentState()
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


double OnScreenJoystickViewImpl::getPosition(int axis) const
{
    if(axis >=0 && axis < static_cast<int>(axisPositions.size())) {
        return axisPositions[axis];
    }
    return 0.0;
}


bool OnScreenJoystickViewImpl::getButtonState(int button) const
{
    if(button >= 0 && button < static_cast<int>(buttonStates.size())) {
        return buttonStates[button];
    }
    return false;
}


bool OnScreenJoystickViewImpl::isActive() const
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


SignalProxy<void(int id, bool isPressed)> OnScreenJoystickViewImpl::sigButton()
{
    return sigButton_;
}


SignalProxy<void(int id, double position)> OnScreenJoystickViewImpl::sigAxis()
{
    return sigAxis_;
}


void OnScreenJoystickView::onAttachedMenuRequest(MenuManager& menuManager)
{
    auto screenCheck = menuManager.addCheckItem(_("On-screen Joystick"));
    screenCheck->setChecked(impl->isOnScreenJoystickEnabled);
    screenCheck->sigToggled().connect([&](bool on){
        impl->isOnScreenJoystickEnabled = on;
        impl->topWidget->setCurrentIndex(on ? 1 : 0);
    });
}


bool OnScreenJoystickView::storeState(Archive& archive)
{
    return true;
}


bool OnScreenJoystickView::restoreState(const Archive& archive)
{
    return true;
}
