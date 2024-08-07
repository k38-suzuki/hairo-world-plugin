/**
   @author Kenta Suzuki
*/

#include "NetworkEmulator.h"
#include <cnoid/Button>
#include <cnoid/ComboBox>
#include <cnoid/Dialog>
#include <cnoid/ExtensionManager>
#include <cnoid/MainMenu>
#include <cnoid/Separator>
#include <cnoid/SpinBox>
#include <QBoxLayout>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QLabel>
#include "gettext.h"
#include "NetEm.h"

using namespace cnoid;

namespace {

NetworkEmulator* emulatorInstance = nullptr;

}

namespace cnoid {

class NetworkEmulator::Impl : public Dialog
{
public:

    NetEmPtr emulator;
    ComboBox* interfaceCombo;
    ComboBox* ifbdeviceCombo;
    DoubleSpinBox* delaySpins[2];
    DoubleSpinBox* rateSpins[2];
    DoubleSpinBox* lossSpins[2];
    PushButton* startButton;

    Impl();
    void onStartButtonToggled(const bool& on);
};

}


void NetworkEmulator::initializeClass(ExtensionManager* ext)
{
    if(!emulatorInstance) {
        emulatorInstance = ext->manage(new NetworkEmulator);

        MainMenu::instance()->add_Tools_Item(
            _("NetworkEmulator"), []() { emulatorInstance->impl->show(); });
    }
}



NetworkEmulator::NetworkEmulator()
{
    impl = new Impl;
}


NetworkEmulator::Impl::Impl()
{
    setWindowTitle(_("Network Emulator"));

    emulator = new NetEm;

    static const char* label[] = {
        _("Interface"), _("IFB Device"),
        _("Inbound Delay [ms]"), _("Inbound Rate [kbit/s]"),_("Inbound Loss [%]"),
        _("Outbound Delay [ms]"), _("Outbound Rate [kbit/s]"), _("Outbound Loss [%]")
    };

    QGridLayout* gbox = new QGridLayout;
    for(int i = 0; i < 8; ++i) {
        gbox->addWidget(new QLabel(label[i]), i, 0);
    }

    interfaceCombo = new ComboBox;
    for(int i = 0; i < emulator->interfaces().size(); ++i) {
        interfaceCombo->addItem(emulator->interfaces()[i].c_str());
    }
    ifbdeviceCombo = new ComboBox;
    ifbdeviceCombo->addItems(QStringList() << "ifb0" << "ifb1");
    ifbdeviceCombo->setCurrentIndex(1);

    for(int i = 0; i < 2; ++i) {
        delaySpins[i] = new DoubleSpinBox;
        delaySpins[i]->setRange(0.0, 100000.0);
        rateSpins[i] = new DoubleSpinBox;
        rateSpins[i]->setRange(0.0, 11000000.0);
        lossSpins[i] = new DoubleSpinBox;
        lossSpins[i]->setRange(0.0, 100.0);
    }

    gbox->addWidget(interfaceCombo, 0, 1);
    gbox->addWidget(ifbdeviceCombo, 1, 1);
    gbox->addWidget(delaySpins[0], 2, 1);
    gbox->addWidget(rateSpins[0], 3, 1);
    gbox->addWidget(lossSpins[0], 4, 1);
    gbox->addWidget(delaySpins[1], 5, 1);
    gbox->addWidget(rateSpins[1], 6, 1);
    gbox->addWidget(lossSpins[1], 7, 1);

    QDialogButtonBox* buttonBox = new QDialogButtonBox(this);
    startButton = new PushButton(_("&Start"));
    startButton->setCheckable(true);
    buttonBox->addButton(startButton, QDialogButtonBox::ActionRole);
    startButton->sigToggled().connect([&](bool on){ onStartButtonToggled(on); });

    auto vbox = new QVBoxLayout;
    vbox->addLayout(gbox);
    vbox->addStretch();
    vbox->addWidget(new HSeparator);
    vbox->addWidget(buttonBox);
    setLayout(vbox);
}


NetworkEmulator::~NetworkEmulator()
{
    delete impl;
}


void NetworkEmulator::Impl::onStartButtonToggled(const bool& on)
{
    if(on) {
        startButton->setText(_("&Stop"));
        emulator->start(interfaceCombo->currentIndex(), ifbdeviceCombo->currentIndex());
        for(int i = 0; i < 2; ++i) {
            emulator->setDelay(i, delaySpins[i]->value());
            emulator->setRate(i, rateSpins[i]->value());
            emulator->setLoss(i, lossSpins[i]->value());
        }
        emulator->update();
    } else {
        startButton->setText(_("&Start"));
        emulator->stop();
    }
}
