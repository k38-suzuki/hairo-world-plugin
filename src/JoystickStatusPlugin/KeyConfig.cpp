/**
   \file
   \author Kenta Suzuki
*/

#include "KeyConfig.h"
#include <cnoid/Archive>
#include <cnoid/Button>
#include <cnoid/ComboBox>
#include <cnoid/Dialog>
#include <cnoid/Joystick>
#include <cnoid/Separator>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>
#include "gettext.h"

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

class KeyConfigImpl : public Dialog
{
public:
    KeyConfigImpl(KeyConfig* self);
    KeyConfig* self;

    ComboBox* axisCombos[Joystick::NUM_STD_AXES];
    ComboBox* buttonCombos[Joystick::NUM_STD_BUTTONS];
};

}


KeyConfig::KeyConfig()
{
    impl = new KeyConfigImpl(this);
}


KeyConfigImpl::KeyConfigImpl(KeyConfig* self)
    : self(self)
{
    setWindowTitle(_("KeyConfig"));

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

//    QDialogButtonBox* buttonBox = new QDialogButtonBox(this);
//    PushButton* okButton = new PushButton(_("&Ok"));
//    buttonBox->addButton(okButton, QDialogButtonBox::AcceptRole);
//    connect(buttonBox, &QDialogButtonBox::accepted, [this](){ this->accept(); });

    QHBoxLayout* hbox = new QHBoxLayout;
    hbox->addLayout(avbox);
    hbox->addWidget(new VSeparator);
    hbox->addLayout(bvbox);
    QVBoxLayout* vbox = new QVBoxLayout;
    vbox->addLayout(hbox);
//    vbox->addWidget(new HSeparator);
//    vbox->addWidget(buttonBox);
    setLayout(vbox);
}


KeyConfig::~KeyConfig()
{
    delete impl;
}


int KeyConfig::axisID(const int& axis)
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


int KeyConfig::buttonID(const int& button)
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


void KeyConfig::showConfig()
{
    impl->show();
}
