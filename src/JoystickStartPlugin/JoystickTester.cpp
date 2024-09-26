/**
   @author Kenta Suzuki
*/

#include "JoystickTester.h"
#include <cnoid/Action>
#include <cnoid/Buttons>
#include <cnoid/Dialog>
#include <cnoid/ExtensionManager>
#include <cnoid/Format>
#include <cnoid/JoystickCapture>
#include <cnoid/MainMenu>
#include <cnoid/Separator>
#include <cnoid/HamburgerMenu>
#include <QBoxLayout>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QProgressBar>
#include <vector>
#include "VirtualJoystickWidget.h"
#include "gettext.h"

using namespace std;
using namespace cnoid;

namespace {

JoystickTester* testerInstance = nullptr;
VirtualJoystickWidget* joystickInstance = nullptr;

}

namespace cnoid {

class JoystickTester::Impl : public Dialog
{
public:

    Impl();

    void onAxis(const int& id, const double& position);
    void onButton(const int& id, bool isPressed);

    vector<QProgressBar*> bars;
    vector<PushButton*> buttons;

    JoystickCapture joystick;
};

}


void JoystickTester::initializeClass(ExtensionManager* ext)
{
    if(!testerInstance) {
        testerInstance = ext->manage(new JoystickTester);

        const QIcon icon = QIcon(":/GoogleMaterialSymbols/icon/joystick_24dp_5F6368_FILL1_wght400_GRAD0_opsz24.svg");
        auto action = new Action;
        action->setText(_("Joystick Tester"));
        action->setIcon(icon);
        action->setToolTip(_("Show the joystick tester"));
        action->sigTriggered().connect([&](){ testerInstance->impl->show(); });
        HamburgerMenu::instance()->addAction(action);
    }

    if(!joystickInstance) {
        joystickInstance = ext->manage(new VirtualJoystickWidget);
        joystickInstance->setWindowFlags(Qt::WindowStaysOnTopHint);

        const QIcon icon = QIcon(":/GoogleMaterialSymbols/icon/videogame_asset_24dp_5F6368_FILL1_wght400_GRAD0_opsz24.svg");
        auto action = new Action;
        action->setText(_("Virtual Joystick2"));
        action->setIcon(icon);
        action->setToolTip(_("Show the virtual joystick"));
        action->sigTriggered().connect([&](){ joystickInstance->show(); });
        HamburgerMenu::instance()->addAction(action);
    }
}


JoystickTester* JoystickTester::instance()
{
    return testerInstance;
}


JoystickTester::JoystickTester()
{
    impl = new Impl;
}


JoystickTester::Impl::Impl()
    : Dialog()
{
    setWindowTitle(_("Joystick Tester"));

    joystick.setDevice("/dev/input/js0");

    joystick.sigAxis().connect(
        [&](int id, double position){ onAxis(id, position); });

    joystick.sigButton().connect(
        [&](int id, bool isPressed){ onButton(id, isPressed); });

    QGroupBox* gbox2 = new QGroupBox(_("Axes"));
    auto vbox1 = new QVBoxLayout;
    QGridLayout* gbox1 = new QGridLayout;
    for(int i = 0; i < joystick.numAxes(); ++i) {
        QProgressBar* bar = new QProgressBar;
        bar->setValue(0);
        bar->setRange(-100, 100);
        bar->setFormat(formatC("{0:.3}%", 0.0).c_str());
        bars.push_back(bar);
        const string label = "Axis " + to_string(i) + ":";
        gbox1->addWidget(new QLabel(label.c_str()), i, 0);
        gbox1->addWidget(bar, i, 1);
    }
    vbox1->addLayout(gbox1);
    vbox1->addStretch();
    gbox2->setLayout(vbox1);

    QGroupBox* gbox3 = new QGroupBox(_("Buttons"));
    auto vbox2 = new QVBoxLayout;
    for(int i = 0; i < joystick.numButtons(); ++i) {
        PushButton* button = new PushButton(to_string(i).c_str());
        buttons.push_back(button);
        vbox2->addWidget(button);
    }
    vbox2->addStretch();
    gbox3->setLayout(vbox2);

    auto hbox = new QHBoxLayout;
    hbox->addWidget(gbox2);
    hbox->addWidget(gbox3);

    auto okButton = new QPushButton(_("&Ok"));
    okButton->setDefault(true);
    auto buttonBox = new QDialogButtonBox(this);
    buttonBox->addButton(okButton, QDialogButtonBox::AcceptRole);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));

    auto vbox = new QVBoxLayout;
    vbox->addLayout(hbox);
    vbox->addStretch();
    vbox->addWidget(new HSeparator);
    vbox->addWidget(buttonBox);
    setLayout(vbox);
}


JoystickTester::~JoystickTester()
{
    delete impl;
}


void JoystickTester::Impl::onAxis(const int& id, const double& position)
{
    QProgressBar* bar = bars[id];
    double value = 100.0 * position;
    bar->setValue(value);
    bar->setFormat(formatC("{0:.3}%", value).c_str());
}


void JoystickTester::Impl::onButton(const int& id, bool isPressed)
{
    PushButton* button = buttons[id];
    QPalette palette;
    if(isPressed) {
        palette.setColor(QPalette::Button, QColor(Qt::red));
    }
    button->setPalette(palette);
}