/**
   @author Kenta Suzuki
*/

#include "NetworkEmulator.h"
#include <cnoid/Button>
#include <cnoid/ComboBox>
#include <cnoid/Dialog>
#include <cnoid/MenuManager>
#include <cnoid/Separator>
#include <cnoid/SpinBox>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QLabel>
#include <QVBoxLayout>
#include "gettext.h"
#include "NetEm.h"

using namespace cnoid;

namespace {

NetworkEmulator* emulatorInstance = nullptr;

}

namespace cnoid {

class NetworkEmulatorImpl : public Dialog
{
public:
    NetworkEmulatorImpl(NetworkEmulator* self);
    NetworkEmulator* self;

    NetEmPtr emulator;
    ComboBox* interfaceCombo;
    ComboBox* ifbdeviceCombo;
    DoubleSpinBox* delaySpins[2];
    DoubleSpinBox* rateSpins[2];
    DoubleSpinBox* lossSpins[2];
    PushButton* startButton;

    void onStartButtonToggled(const bool& on);
};

}


NetworkEmulator::NetworkEmulator()
{
    impl = new NetworkEmulatorImpl(this);
}


NetworkEmulatorImpl::NetworkEmulatorImpl(NetworkEmulator* self)
    : self(self)
{
    setWindowTitle(_("Network Emulator"));

    emulator = new NetEm;

    static const char* label[] = {
        _("Interface"), _("IFB Device"),
        _("InboundDelay"), _("InboundRate"),_("InboundLoss"),
        _("OutboundDelay"), _("OutboundRate"), _("OutboundLoss")
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
    const QStringList items = { _("ifb0"), _("ifb1") };
    ifbdeviceCombo->addItems(items);
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

    QVBoxLayout* vbox = new QVBoxLayout;
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


void NetworkEmulator::initializeClass(ExtensionManager* ext)
{
    if(!emulatorInstance) {
        emulatorInstance = ext->manage(new NetworkEmulator);
    }
    MenuManager& mm = ext->menuManager().setPath("/" N_("Tools"));
    mm.addItem(_("NetworkEmulator"))->sigTriggered().connect(
                [&](){ emulatorInstance->impl->show(); });
}


void NetworkEmulatorImpl::onStartButtonToggled(const bool& on)
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
