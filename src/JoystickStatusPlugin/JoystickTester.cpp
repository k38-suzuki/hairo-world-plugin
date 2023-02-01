/**
   \file
   \author Kenta Suzuki
*/

#include "JoystickTester.h"
#include <cnoid/Action>
#include <cnoid/Buttons>
#include <cnoid/Dialog>
#include <cnoid/JoystickCapture>
#include <cnoid/MenuManager>
#include <cnoid/Separator>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QProgressBar>
#include <QVBoxLayout>
#include <vector>
#include <fmt/format.h>
#include "gettext.h"

using namespace cnoid;
using namespace std;

namespace {

JoystickTester* instance_ = nullptr;

}

namespace cnoid {

class JoystickTesterImpl : public Dialog
{
public:
    JoystickTesterImpl(JoystickTester* self);
    JoystickTester* self;

    vector<QProgressBar*> bars;
    vector<PushButton*> buttons;

    JoystickCapture joystick;

    void onAxis(const int& id, const double& position);
    void onButton(const int& id, const bool& isPressed);
};

}


JoystickTester::JoystickTester()
{
    impl = new JoystickTesterImpl(this);
}


JoystickTesterImpl::JoystickTesterImpl(JoystickTester* self)
    : self(self)
{
    setWindowTitle(_("JoystickTest"));

    joystick.setDevice("/dev/input/js0");

    joystick.sigAxis().connect(
        [&](int id, double position){ onAxis(id, position); });

    joystick.sigButton().connect(
        [&](int id, bool isPressed){ onButton(id, isPressed); });

    QGroupBox* gbox0 = new QGroupBox(_("Axes"));
    QVBoxLayout* vbox0 = new QVBoxLayout;
    QGridLayout* grid = new QGridLayout;
    for(int i = 0; i < joystick.numAxes(); ++i) {
        QProgressBar* bar = new QProgressBar;
        bar->setValue(0);
        bar->setRange(-100, 100);
        bar->setFormat(fmt::format("{0:.3}", 0.0).c_str());
        bars.push_back(bar);
        const string label = "Axis " + to_string(i) + ":";
        grid->addWidget(new QLabel(label.c_str()), i, 0);
        grid->addWidget(bar, i, 1);
    }
    vbox0->addLayout(grid);
    vbox0->addStretch();
    gbox0->setLayout(vbox0);

    QGroupBox* gbox1 = new QGroupBox(_("Buttons"));
    QVBoxLayout* vbox1 = new QVBoxLayout;
    for(int i = 0; i < joystick.numButtons(); ++i) {
        PushButton* button = new PushButton(to_string(i).c_str());
        buttons.push_back(button);
        vbox1->addWidget(button);
    }
    vbox1->addStretch();
    gbox1->setLayout(vbox1);

    QHBoxLayout* hbox = new QHBoxLayout;
    hbox->addWidget(gbox0);
    hbox->addWidget(gbox1);

    auto okButton = new QPushButton(_("&Ok"));
    okButton->setDefault(true);
    auto buttonBox = new QDialogButtonBox(this);
    buttonBox->addButton(okButton, QDialogButtonBox::AcceptRole);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));

    QVBoxLayout* vbox2 = new QVBoxLayout;
    vbox2->addLayout(hbox);
    vbox2->addWidget(new HSeparator);
    vbox2->addWidget(buttonBox);
    setLayout(vbox2);
}


JoystickTester::~JoystickTester()
{
    delete impl;
}


void JoystickTester::initializeClass(ExtensionManager* ext)
{
    if(!instance_) {
        instance_ = ext->manage(new JoystickTester);
    }

    MenuManager& mm = ext->menuManager().setPath("/" N_("Tools"));    
    mm.addItem(_("JoystickTest"))->sigTriggered().connect(
        [&](){ instance_->impl->show(); });
}


void JoystickTesterImpl::onAxis(const int& id, const double& position)
{
    QProgressBar* bar = bars[id];
    double value = 100.0 * position;
    bar->setValue(value);
    bar->setFormat(fmt::format("{0:.3}", value).c_str());
}


void JoystickTesterImpl::onButton(const int& id, const bool& isPressed)
{
    PushButton* button = buttons[id];
    QPalette palette;
    if(isPressed) {
        palette.setColor(QPalette::Button, QColor(Qt::red));
    }
    button->setPalette(palette);
}
