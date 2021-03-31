/**
   \file
   \author  Kenta Suzuki
*/

#include "JoystickStatusView.h"
#include <cnoid/ViewManager>
#include <cnoid/Buttons>
#include <cnoid/ExtJoystick>
#include <cnoid/Joystick>
#include <cnoid/JoystickCapture>
#include <QBoxLayout>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QKeyEvent>
#include <QProgressBar>
#include <QVBoxLayout>
#include <thread>
#include <mutex>
#include <cmath>
#include "gettext.h"

using namespace std;
using namespace cnoid;

namespace {

const bool TRACE_FUNCTIONS = false;

enum {
    ARROW_UP, ARROW_DOWN, ARROW_LEFT, ARROW_RIGHT,
    L_AXIS_UP, L_AXIS_DOWN, L_AXIS_LEFT, L_AXIS_RIGHT,
    R_AXIS_UP, R_AXIS_DOWN, R_AXIS_LEFT, R_AXIS_RIGHT,
    L_AXIS_TRIGGER, R_AXIS_TRIGGER,
    BUTTON_A, BUTTON_B, BUTTON_X, BUTTON_Y,
    BUTTON_L, BUTTON_R, BUTTON_SELECT, BUTTON_START,
    BUTTON_L_STICK, BUTTON_R_STICK,
    BUTTON_LOGO, NUM_JOYSTICK_ELEMENTS
};

const QString names[] = {
    "L_STICK_H_AXIS",
    "L_STICK_V_AXIS",
    "R_STICK_H_AXIS",
    "R_STICK_V_AXIS",
    "DIR_PAD_H_AXIS",
    "DIR_PAD_V_AXIS",
    "L_TRIGGER_AXIS",
    "R_TRIGGER_AXIS"
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

    { "^", 0, 1, true, -1.0, 5, Qt::Key_Up },
    { "v", 2, 1, true,  1.0, 5, Qt::Key_Down },
    { "<", 1, 0, true, -1.0, 4, Qt::Key_Left },
    { ">", 1, 2, true,  1.0, 4, Qt::Key_Right },

    { "E", 3, 3, true, -1.0, 1, Qt::Key_E },
    { "D", 5, 3, true,  1.0, 1, Qt::Key_D },
    { "S", 4, 2, true, -1.0, 0, Qt::Key_S },
    { "F", 4, 4, true,  1.0, 0, Qt::Key_F },

    { "I", 3, 8, true, -1.0, 3, Qt::Key_I },
    { "K", 5, 8, true,  1.0, 3, Qt::Key_K },
    { "J", 4, 7, true, -1.0, 2, Qt::Key_J },
    { "L", 4, 9, true,  1.0, 2, Qt::Key_L },

    { "Q", 3, 1, true,  1.0, 6, Qt::Key_Q },
    { "P", 3, 10, true, 1.0, 7, Qt::Key_P },

    { "A", 2, 10, false, 1.0, 0, Qt::Key_A },
    { "B", 1, 11, false, 1.0, 1, Qt::Key_B },
    { "X", 1,  9, false, 1.0, 2, Qt::Key_X },
    { "Y", 0, 10, false, 1.0, 3, Qt::Key_Y },

    { "W", 3, 2, false, 1.0, 4, Qt::Key_W },
    { "O", 3, 9, false, 1.0, 5, Qt::Key_O },
    { "G", 4, 5, false, 1.0, 6, Qt::Key_G },
    { "H", 4, 6, false, 1.0, 7, Qt::Key_H },

    { "C", 4, 3, false, 1.0, 8, Qt::Key_C },
    { "M", 4, 8, false, 1.0, 9, Qt::Key_M },
    { " ", 3, 5, false, 1.0, 10, Qt::Key_Space }
};
    
}

namespace cnoid {

class JoystickStatusViewImpl : public ExtJoystick
{
public:
    JoystickStatusView* self;
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
    QProgressBar bars[Joystick::NUM_STD_AXES];

    JoystickStatusViewImpl(JoystickStatusView* self);
    ~JoystickStatusViewImpl();
    bool onKeyStateChanged(int key, bool on);
    void onButtonPressed(int index);
    void onButtonReleased(int index);
    
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


void JoystickStatusView::initializeClass(ExtensionManager* ext)
{
    ext->viewManager().registerClass<JoystickStatusView>(
        "JoystickStatusView", N_("Joystick Status"), ViewManager::SINGLE_OPTIONAL);
}


JoystickStatusView::JoystickStatusView()
{
    impl = new JoystickStatusViewImpl(this);
}


JoystickStatusViewImpl::JoystickStatusViewImpl(JoystickStatusView* self)
    : self(self),
      keyValues(NUM_JOYSTICK_ELEMENTS, 0.0)
{
    self->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    self->setDefaultLayoutArea(View::BOTTOM);
    self->setFocusPolicy(Qt::WheelFocus);
    
    for(int i=0; i < NUM_JOYSTICK_ELEMENTS; ++i){
        ButtonInfo& info = buttonInfo[i];
        ToolButton& button = buttons[i];
        button.setText(info.label);
        grid.addWidget(&buttons[i], info.row, info.column);
        keyToButtonMap[info.key] = i;
        if(info.isAxis){
            if(info.id >= static_cast<int>(axisPositions.size())){
                axisPositions.resize(info.id + 1, 0.0);
            }
        } else {
            if(info.id >= static_cast<int>(buttonStates.size())){
                buttonStates.resize(info.id + 1, false);
            }
        }

        button.sigPressed().connect([=](){ onButtonPressed(i); });
        button.sigReleased().connect([=](){ onButtonReleased(i); });
    }

    joystick.setDevice("/dev/input/js0");
    joystick.sigButton().connect([&](int id, bool isPressed) {
        for(int i=0; i < NUM_JOYSTICK_ELEMENTS; ++i){
            ButtonInfo& info = buttonInfo[i];
            if(!info.isAxis){
                if(id == info.id) {
                    QPalette palette;
                    if(isPressed) {
                      palette.setColor(QPalette::Button, QColor(Qt::red));
                    }
                    buttons[i].setPalette(palette);
                }
            }
        }
    });

    joystick.sigAxis().connect([&](int id, double position) {
        for(int i=0; i < NUM_JOYSTICK_ELEMENTS; ++i){
            ButtonInfo& info = buttonInfo[i];
            if(info.isAxis){
                if(id == info.id) {
                    QPalette palette;
                    if(position < 0.0 && info.activeValue < 0.0) {
                        palette.setColor(QPalette::Button, QColor(Qt::red));
                    } else if(position > 0.0 && info.activeValue > 0.0) {
                        palette.setColor(QPalette::Button, QColor(Qt::red));
                    }
                    buttons[i].setPalette(palette);
                }
            }
        }
        bars[id].setValue(fabs(100.0 * position));
        if(position < 0.0) {
            bars[id].setStyleSheet("QProgressBar { text-align: center; } QProgressBar::chunk { background-color: magenta; }") ;
        } else {
            bars[id].setStyleSheet("QProgressBar { text-align: center; } QProgressBar::chunk { background-color: cyan; }") ;
        }
    });

    QGridLayout* gbox = new QGridLayout();
    for(int i = 0; i < Joystick::NUM_STD_AXES; ++i) {
        bars[i].setValue(0);
        gbox->addWidget(new QLabel(names[i]), i, 0);
        gbox->addWidget(&bars[i], i, 1);
    }

    QHBoxLayout* hbox = new QHBoxLayout;
    hbox->addStretch();
    hbox->addLayout(&grid);
    hbox->addStretch();
    QVBoxLayout* vbox = new QVBoxLayout;
    vbox->addStretch();
    vbox->addLayout(hbox);
    vbox->addStretch();

    QHBoxLayout* mainbox = new QHBoxLayout();
    mainbox->addLayout(vbox);
    mainbox->addLayout(gbox);
    self->setLayout(mainbox);

    ExtJoystick::registerJoystick("JoystickStatusView", this);
}


JoystickStatusView::~JoystickStatusView()
{
    delete impl;
}


JoystickStatusViewImpl::~JoystickStatusViewImpl()
{

}


void JoystickStatusView::keyPressEvent(QKeyEvent* event)
{
    if(!impl->onKeyStateChanged(event->key(), true)){
        View::keyPressEvent(event);
    }
}


void JoystickStatusView::keyReleaseEvent(QKeyEvent* event)
{
    if(!impl->onKeyStateChanged(event->key(), false)){
        View::keyPressEvent(event);
    }
}


bool JoystickStatusViewImpl::onKeyStateChanged(int key, bool on)
{
    KeyToButtonMap::iterator p = keyToButtonMap.find(key);
    if(p == keyToButtonMap.end()){
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

        QPalette palette;
        if(on) {
            palette.setColor(QPalette::Button, QColor(Qt::red));
        }
        button.setPalette(palette);
        if(info.isAxis) {
            if(on) {
                bars[info.id].setValue(fabs(100.0 * info.activeValue));
                if(info.activeValue < 0.0) {
                    bars[info.id].setStyleSheet("QProgressBar { text-align: center; } QProgressBar::chunk { background-color: magenta; }") ;
                } else {
                    bars[info.id].setStyleSheet("QProgressBar { text-align: center; } QProgressBar::chunk { background-color: cyan; }") ;
                }
            } else {
                bars[info.id].setValue(0);
            }
        }
    }
    return true;
}


void JoystickStatusViewImpl::onButtonPressed(int index)
{
    ButtonInfo& info = buttonInfo[index];
    std::lock_guard<std::mutex> lock(mutex);
    keyValues[index] = info.activeValue;

    QPalette palette;
    palette.setColor(QPalette::Button, QColor(Qt::red));
    buttons[index].setPalette(palette);
    if(info.isAxis) {
        bars[info.id].setValue(fabs(100.0 * info.activeValue));
        if(info.activeValue < 0.0) {
            bars[info.id].setStyleSheet("QProgressBar { text-align: center; } QProgressBar::chunk { background-color: magenta; }") ;
        } else {
            bars[info.id].setStyleSheet("QProgressBar { text-align: center; } QProgressBar::chunk { background-color: cyan; }") ;
        }
    }
}


void JoystickStatusViewImpl::onButtonReleased(int index)
{
    std::lock_guard<std::mutex> lock(mutex);
    keyValues[index] = 0.0;

    QPalette palette;
    buttons[index].setPalette(palette);

    ButtonInfo& info = buttonInfo[index];
    if(info.isAxis) {
        bars[info.id].setValue(0);
    }
}


int JoystickStatusViewImpl::numAxes() const
{
    return axisPositions.size();
}


int JoystickStatusViewImpl::numButtons() const
{
    return buttonStates.size();
}


bool JoystickStatusViewImpl::readCurrentState()
{
    std::fill(axisPositions.begin(), axisPositions.end(), 0.0);

    {
        std::lock_guard<std::mutex> lock(mutex);
        for(int i=0; i < NUM_JOYSTICK_ELEMENTS; ++i){
            ButtonInfo& info = buttonInfo[i];
            if(info.isAxis){
                axisPositions[info.id] += keyValues[i];
            } else {
                buttonStates[info.id] = keyValues[i];
            }
        }
    }
    return true;
}


double JoystickStatusViewImpl::getPosition(int axis) const
{
    if(axis >=0 && axis < static_cast<int>(axisPositions.size())){
        return axisPositions[axis];
    }
    return 0.0;
}


bool JoystickStatusViewImpl::getButtonState(int button) const
{
    if(button >= 0 && button < static_cast<int>(buttonStates.size())){
        return buttonStates[button];
    }
    return false;
}


bool JoystickStatusViewImpl::isActive() const
{
    for(size_t i=0; i < axisPositions.size(); ++i){
        if(axisPositions[i] != 0.0){
            return true;
        }
    }
    for(size_t i=0; i < buttonStates.size(); ++i){
        if(buttonStates[i]){
            return true;
        }
    }
    return false;
}


SignalProxy<void(int id, bool isPressed)> JoystickStatusViewImpl::sigButton()
{
    return sigButton_;
}


SignalProxy<void(int id, double position)> JoystickStatusViewImpl::sigAxis()
{
    return sigAxis_;
}


bool JoystickStatusView::storeState(Archive&)
{
    return true;
}


bool JoystickStatusView::restoreState(const Archive&)
{
    return true;
}
