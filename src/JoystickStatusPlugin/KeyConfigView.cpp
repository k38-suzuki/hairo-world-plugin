/**
   \file
   \author Kenta Suzuki
*/

#include "KeyConfigView.h"
#include <cnoid/Archive>
#include <cnoid/ComboBox>
#include <cnoid/Joystick>
#include <cnoid/Separator>
#include <cnoid/ViewManager>
#include <cnoid/Widget>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QScrollArea>
#include <QVBoxLayout>

using namespace cnoid;
using namespace std;

namespace {

struct ItemInfo {
    const char* name;
    int id;
};

ItemInfo axisInfo[] = {
    { "L_STICK_H_AXIS",                 Joystick::L_STICK_H_AXIS },
    { "L_STICK_V_AXIS",                 Joystick::L_STICK_V_AXIS },
    { "R_STICK_H_AXIS",                 Joystick::R_STICK_H_AXIS },
    { "R_STICK_V_AXIS",                 Joystick::R_STICK_V_AXIS },
    { "DIRECTIONAL_PAD_H_AXIS", Joystick::DIRECTIONAL_PAD_H_AXIS },
    { "DIRECTIONAL_PAD_V_AXIS", Joystick::DIRECTIONAL_PAD_V_AXIS },
    { "L_TRIGGER_AXIS",                 Joystick::L_TRIGGER_AXIS },
    { "R_TRIGGER_AXIS",                 Joystick::R_TRIGGER_AXIS }
};

ItemInfo buttonInfo[] = {
    { "A_BUTTON",             Joystick::A_BUTTON },
    { "B_BUTTON",             Joystick::B_BUTTON },
    { "X_BUTTON",             Joystick::X_BUTTON },
    { "Y_BUTTON",             Joystick::Y_BUTTON },
    { "L_BUTTON",             Joystick::L_BUTTON },
    { "R_BUTTON",             Joystick::R_BUTTON },
    { "SELECT_BUTTON",   Joystick::SELECT_BUTTON },
    { "START_BUTTON",     Joystick::START_BUTTON },
    { "L_STICK_BUTTON", Joystick::L_STICK_BUTTON },
    { "R_STICK_BUTTON", Joystick::R_STICK_BUTTON },
    { "LOGO_BUTTON",       Joystick::LOGO_BUTTON }
};

}

namespace cnoid {

class KeyConfigViewImpl
{
public:
    KeyConfigViewImpl(KeyConfigView* self);
    KeyConfigView* self;

    QScrollArea scrollArea;
    ComboBox* axisCombos[Joystick::NUM_STD_AXES];
    ComboBox* buttonCombos[Joystick::NUM_STD_BUTTONS];

    bool storeState(Archive& archive);
    bool restoreState(const Archive& archive);
};

}


KeyConfigView::KeyConfigView()
{
    impl = new KeyConfigViewImpl(this);
}


KeyConfigViewImpl::KeyConfigViewImpl(KeyConfigView* self)
    : self(self)
{
    self->setDefaultLayoutArea(View::BottomCenterArea);

    Widget* topWidget = new Widget;
    topWidget->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);

    scrollArea.setStyleSheet("QScrollArea {background: transparent;}");
    scrollArea.setFrameShape(QFrame::NoFrame);
    scrollArea.setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea.setWidget(topWidget);
    topWidget->setAutoFillBackground(false);
    QVBoxLayout* baseLayout = new QVBoxLayout;
    scrollArea.setWidgetResizable(true);
    baseLayout->addWidget(&scrollArea);
    self->setLayout(baseLayout);

    QGridLayout* agbox = new QGridLayout;
    agbox->addWidget(new QLabel("Axis"), 0, 0);
    agbox->addWidget(new QLabel("ID"), 0, 1);
    for(int i = 0; i < Joystick::NUM_STD_AXES; ++i) {
        axisCombos[i] = new ComboBox;
        for(int j = 0; j < Joystick::NUM_STD_AXES; ++j) {
            ItemInfo info = axisInfo[j];
            axisCombos[i]->addItem(info.name);
        }
        axisCombos[i]->setCurrentIndex(i);
        agbox->addWidget(new QLabel(axisInfo[i].name));
        agbox->addWidget(axisCombos[i], i + 1, 1);
    }
    QVBoxLayout* avbox = new QVBoxLayout;
    avbox->addLayout(agbox);
    avbox->addStretch();

    QGridLayout* bgbox = new QGridLayout;
    bgbox->addWidget(new QLabel("Button"), 0, 0);
    bgbox->addWidget(new QLabel("ID"), 0, 1);
    for(int i = 0; i < Joystick::NUM_STD_BUTTONS; ++i) {
        buttonCombos[i] = new ComboBox;
        for(int j = 0; j < Joystick::NUM_STD_BUTTONS; ++j) {
            ItemInfo info = buttonInfo[j];
            buttonCombos[i]->addItem(info.name);
        }
        buttonCombos[i]->setCurrentIndex(i);
        bgbox->addWidget(new QLabel(buttonInfo[i].name));
        bgbox->addWidget(buttonCombos[i], i + 1, 1);
    }
    QVBoxLayout* bvbox = new QVBoxLayout;
    bvbox->addLayout(bgbox);
    bvbox->addStretch();

    QHBoxLayout* hbox = new QHBoxLayout;
    hbox->addLayout(avbox);
    hbox->addWidget(new VSeparator);
    hbox->addLayout(bvbox);
    topWidget->setLayout(hbox);
}


KeyConfigView::~KeyConfigView()
{
    delete impl;
}


void KeyConfigView::initializeClass(ExtensionManager* ext)
{
    ext->viewManager().registerClass<KeyConfigView>(
                ("KeyConfigView"), ("KeyConfig"), ViewManager::SINGLE_OPTIONAL);
}


KeyConfigView* KeyConfigView::instance()
{
    static KeyConfigView* instance_ = ViewManager::findView<KeyConfigView>();
    return instance_;
}


int KeyConfigView::axisID(const int& axis)
{
    int index = 0;
    for(int i = 0; i < Joystick::NUM_STD_AXES; ++i) {
        ItemInfo info = axisInfo[i];
        if(info.id == axis) {
            index = i;
        }
    }
    return impl->axisCombos[index]->currentIndex();
}


int KeyConfigView::buttonID(const int& button)
{
    int index = 0;
    for(int i = 0; i < Joystick::NUM_STD_BUTTONS; ++i) {
        ItemInfo info = buttonInfo[i];
        if(info.id == button) {
            index = i;
        }
    }
    return impl->buttonCombos[index]->currentIndex();
}


bool KeyConfigView::storeState(Archive &archive)
{
    return impl->storeState(archive);
}


bool KeyConfigViewImpl::storeState(Archive& archive)
{
    for(int i = 0; i < Joystick::NUM_STD_AXES; ++i) {
        string key = "axis_id_" + to_string(i);
        archive.write(key, axisCombos[i]->currentIndex());
    }
    for(int i = 0; i < Joystick::NUM_STD_BUTTONS; ++i) {
        string key = "button_id_" + to_string(i);
        archive.write(key, buttonCombos[i]->currentIndex());
    }
    return true;
}


bool KeyConfigView::restoreState(const Archive& archive)
{
    return impl->restoreState(archive);
}


bool KeyConfigViewImpl::restoreState(const Archive& archive)
{
    for(int i = 0; i < Joystick::NUM_STD_AXES; ++i) {
        string key = "axis_id_" + to_string(i);
        axisCombos[i]->setCurrentIndex(archive.get(key, 0));
    }
    for(int i = 0; i < Joystick::NUM_STD_BUTTONS; ++i) {
        string key = "button_id_" + to_string(i);
        buttonCombos[i]->setCurrentIndex(archive.get(key, 0));
    }
    return true;
}
