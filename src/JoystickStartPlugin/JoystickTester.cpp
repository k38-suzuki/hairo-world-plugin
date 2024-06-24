/**
   @author Kenta Suzuki
*/

#include "JoystickTester.h"
#include <cnoid/Action>
#include <cnoid/Button>
#include <cnoid/Dialog>
#include <cnoid/ExtensionManager>
#include <cnoid/JoystickCapture>
#include <cnoid/MainMenu>
#include <cnoid/MainWindow>
#include <cnoid/MenuManager>
#include <cnoid/Separator>
#include <cnoid/ToolBar>
#include <QBoxLayout>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QProgressBar>
#include <vector>
#include <fmt/format.h>
#include "gettext.h"

using namespace std;
using namespace cnoid;

namespace {

JoystickTester* testerInstance = nullptr;

}

namespace cnoid {

class JoystickTester::Impl : public Dialog
{
public:

    vector<QProgressBar*> bars;
    vector<PushButton*> buttons;

    JoystickCapture joystick;

    Impl();

    void onAxis(const int& id, const double& position);
    void onButton(const int& id, const bool& isPressed);
};

}


void JoystickTester::initializeClass(ExtensionManager* ext)
{
    if(!testerInstance) {
        testerInstance = ext->manage(new JoystickTester);

        MainMenu::instance()->add_Tools_Item(
            _("Joystick Tester"), [](){ testerInstance->impl->show(); });

        // vector<ToolBar*> toolBars = MainWindow::instance()->toolBars();
        // for(auto& bar : toolBars) {
        //     if(bar->name() == "FileBar") {
        //         auto button1 = bar->addButton(QIcon::fromTheme("applications-games"));
        //         button1->setToolTip(_("Show the joystick tester"));
        //         button1->sigClicked().connect([&](){ testerInstance->impl->show(); });
        //     }
        // }
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
{
    setWindowTitle(_("JoystickTester"));

    joystick.setDevice("/dev/input/js0");

    joystick.sigAxis().connect(
        [&](int id, double position){ onAxis(id, position); });

    joystick.sigButton().connect(
        [&](int id, bool isPressed){ onButton(id, isPressed); });

    QGroupBox* gbox0 = new QGroupBox(_("Axes"));
    auto vbox0 = new QVBoxLayout;
    QGridLayout* grid = new QGridLayout;
    for(int i = 0; i < joystick.numAxes(); ++i) {
        QProgressBar* bar = new QProgressBar;
        bar->setValue(0);
        bar->setRange(-100, 100);
        bar->setFormat(fmt::format("{0:.3}%", 0.0).c_str());
        bars.push_back(bar);
        const string label = "Axis " + to_string(i) + ":";
        grid->addWidget(new QLabel(label.c_str()), i, 0);
        grid->addWidget(bar, i, 1);
    }
    vbox0->addLayout(grid);
    vbox0->addStretch();
    gbox0->setLayout(vbox0);

    QGroupBox* gbox1 = new QGroupBox(_("Buttons"));
    auto vbox1 = new QVBoxLayout;
    for(int i = 0; i < joystick.numButtons(); ++i) {
        PushButton* button = new PushButton(to_string(i).c_str());
        buttons.push_back(button);
        vbox1->addWidget(button);
    }
    vbox1->addStretch();
    gbox1->setLayout(vbox1);

    auto hbox = new QHBoxLayout;
    hbox->addWidget(gbox0);
    hbox->addWidget(gbox1);

    auto okButton = new QPushButton(_("&Ok"));
    okButton->setDefault(true);
    auto buttonBox = new QDialogButtonBox(this);
    buttonBox->addButton(okButton, QDialogButtonBox::AcceptRole);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));

    auto vbox2 = new QVBoxLayout;
    vbox2->addLayout(hbox);
    vbox2->addWidget(new HSeparator);
    vbox2->addWidget(buttonBox);
    setLayout(vbox2);
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
    bar->setFormat(fmt::format("{0:.3}%", value).c_str());
}


void JoystickTester::Impl::onButton(const int& id, const bool& isPressed)
{
    PushButton* button = buttons[id];
    QPalette palette;
    if(isPressed) {
        palette.setColor(QPalette::Button, QColor(Qt::red));
    }
    button->setPalette(palette);
}