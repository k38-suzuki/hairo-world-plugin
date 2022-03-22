/**
   \file
   \author Kenta Suzuki
*/

#include "NetworkEmulator.h"
#include <cnoid/Button>
#include <cnoid/ComboBox>
#include <cnoid/Dialog>
#include <cnoid/LineEdit>
#include <cnoid/MenuManager>
#include <cnoid/Separator>
#include <cnoid/SpinBox>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QLabel>
#include <QVBoxLayout>
#include <fmt/format.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include "gettext.h"

using namespace cnoid;
using namespace std;

namespace {

class NetworkEmulatorDialog : public Dialog
{
public:
    NetworkEmulatorDialog();

    NetworkEmulatorPtr emulator;
    ComboBox* interfaceCombo;
    ComboBox* ifbdeviceCombo;
    DoubleSpinBox* delaySpins[2];
    DoubleSpinBox* rateSpins[2];
    DoubleSpinBox* lossSpins[2];
    PushButton* startButton;

    void onStartButtonToggled(const bool& on);
};

NetworkEmulatorDialog* dialog = nullptr;

}


namespace cnoid {

class NetworkEmulatorImpl
{
public:
    NetworkEmulatorImpl(NetworkEmulator* self);
    NetworkEmulator* self;

    vector<string> interfaces;
    vector<string> ifbdevices;
    int currentInterfaceID;
    int currentIfbdeviceID;
    string sourceIP;
    string destinationIP;
    double delays[2];
    double rates[2];
    double losses[2];
    bool isUpdated;
    bool isFinalized;

    void initialize();
    void start(const string& program);
    void clear();
    void update();
    void finalize();
};

}


NetworkEmulator::NetworkEmulator()
{
    impl = new NetworkEmulatorImpl(this);
}


NetworkEmulatorImpl::NetworkEmulatorImpl(NetworkEmulator* self)
    : self(self)
{
    interfaces.clear();
    ifbdevices.clear();
    currentInterfaceID = 0;
    currentIfbdeviceID = 0;
    sourceIP = "0.0.0.0/0";
    destinationIP = "0.0.0.0/0";
    delays[0] = delays[1] = 0.0;
    rates[0] = rates[1] = 0.0;
    losses[0] = losses[1] = 0.0;
    isUpdated = false;
    isFinalized = true;

    // registration of interfaces
    static int IFR_MAX = 10;
    struct ifreq ifr[IFR_MAX];
    struct ifconf ifc;
    int fd = socket(AF_INET, SOCK_DGRAM, 0);

    ifc.ifc_len = sizeof(ifr);
    ifc.ifc_ifcu.ifcu_buf = (char*)ifr;
    ioctl(fd, SIOCGIFCONF, &ifc);
    int nifs = ifc.ifc_len / sizeof(struct ifreq);
    for (int i = 0; i < nifs; ++i) {
        string interface = ifr[i].ifr_name;
        interfaces.push_back(interface);
    }
    ::close(fd);

    // registration of ifbdevices
    ifbdevices.push_back("ifb0");
    ifbdevices.push_back("ifb1");
}


NetworkEmulator::~NetworkEmulator()
{
    if(!impl->isFinalized) {
        impl->finalize();
    }
}


void NetworkEmulator::initialize(ExtensionManager* ext)
{
    if(!dialog) {
        dialog = ext->manage(new NetworkEmulatorDialog);
    }
    ext->menuManager().setPath("/" N_("Tools"));
    ext->menuManager().addItem(_("NetworkEmulator"))->sigTriggered().connect(
                [&](){ dialog->show(); });
}


vector<string>& NetworkEmulator::interfaces() const
{
    return impl->interfaces;
}


void NetworkEmulator::start(const int& interfaceID, const int& ifbdeviceID)
{
    impl->currentInterfaceID = interfaceID;
    impl->currentIfbdeviceID = ifbdeviceID;
    impl->initialize();
}


void NetworkEmulatorImpl::initialize()
{
    if(!isFinalized) {
        finalize();
    }
    start("sudo modprobe ifb;");
    start("sudo modprobe act_mirred;");
    start(fmt::format("sudo ip link set dev {0} up;",
                      ifbdevices[currentIfbdeviceID]));
    isUpdated = false;
    isFinalized = false;
}


void NetworkEmulator::update()
{
    impl->update();
}


void NetworkEmulatorImpl::update()
{
    if(!isFinalized) {
        clear();
        start(fmt::format("sudo tc qdisc add dev {0} ingress handle ffff:;",
                          interfaces[currentInterfaceID]));
        start(fmt::format("sudo tc filter add dev {0} parent ffff: protocol ip u32 match u32 0 0 action mirred egress redirect dev {1};",
                          interfaces[currentInterfaceID], ifbdevices[currentIfbdeviceID]));

        string effects[2];
        for(int i = 0 ; i < 2; ++i) {
            if(delays[i] > 0.0) {
               effects[i] +=  fmt::format(" delay {0:.2f}ms", delays[i]);
            }
            if(rates[i] > 0.0) {
                effects[i] +=  fmt::format(" rate {0:.2f}kbps", rates[i]);
            }
            if(losses[i] > 0.0) {
                effects[i] +=  fmt::format(" loss {0:.2f}%", losses[i]);
            }
        }

        start(fmt::format("sudo tc qdisc add dev {0} root handle 1: prio bands 16 priomap 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0;",
                          ifbdevices[currentIfbdeviceID]));
        start(fmt::format("sudo tc qdisc add dev {0} parent 1:1 handle 10: netem limit 2000;",
                          ifbdevices[currentIfbdeviceID]));
        start(fmt::format("sudo tc qdisc add dev {0} parent 1:2 handle 20: netem limit 2000{1};",
                          ifbdevices[currentIfbdeviceID], effects[0]));
        start(fmt::format("sudo tc filter add dev {0} protocol ip parent 1: prio 2 u32 match ip src {1} match ip dst {2} flowid 1:2;",
                          ifbdevices[currentIfbdeviceID], destinationIP, sourceIP));

        start(fmt::format("sudo tc qdisc add dev {0} root handle 1: prio bands 16 priomap 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0;",
                          interfaces[currentInterfaceID]));
        start(fmt::format("sudo tc qdisc add dev {0} parent 1:1 handle 10: netem limit 2000;",
                          interfaces[currentInterfaceID]));
        start(fmt::format("sudo tc qdisc add dev {0} parent 1:2 handle 20: netem limit 2000{1};",
                          interfaces[currentInterfaceID], effects[1]));
        start(fmt::format("sudo tc filter add dev {0} protocol ip parent 1: prio 2 u32 match ip src {1} match ip dst {2} flowid 1:2;",
                          interfaces[currentInterfaceID], sourceIP, destinationIP));
        isUpdated = true;
    }
}


void NetworkEmulator::stop()
{
    impl->finalize();
}


void NetworkEmulatorImpl::clear()
{
    if(isUpdated) {
        start(fmt::format("sudo tc qdisc del dev {0} ingress;",
                          interfaces[currentInterfaceID]));
        start(fmt::format("sudo tc qdisc del dev {0} root;",
                          ifbdevices[currentIfbdeviceID]));
        start(fmt::format("sudo tc qdisc del dev {0} root;",
                          interfaces[currentInterfaceID]));
        isUpdated = false;
    }
}


void NetworkEmulatorImpl::finalize()
{
    if(!isFinalized) {
        clear();
        start(fmt::format("sudo ip link set dev {0} down;",
                          ifbdevices[currentIfbdeviceID]));
        start("sudo rmmod ifb;");
        isFinalized = true;
    }
}


void NetworkEmulator::setDelay(const int& id, const double& delay)
{
    impl->delays[id] = delay;
}


void NetworkEmulator::setRate(const int& id, const double& rate)
{
    impl->rates[id] = rate;
}


void NetworkEmulator::setLoss(const int &id, const double &loss)
{
    impl->losses[id] = loss;
}


void NetworkEmulator::setSourceIP(const string& sourceIP)
{
    impl->sourceIP = sourceIP;
}


void NetworkEmulator::setDestinationIP(const string& destinationIP)
{
    impl->destinationIP = destinationIP;
}


void NetworkEmulatorImpl::start(const string& program)
{
    int ret = system(program.c_str());
}


NetworkEmulatorDialog::NetworkEmulatorDialog()
{
    setWindowTitle(_("Network Emulator"));

    emulator = new NetworkEmulator;

    static const char* labels[] = {
        _("Interface"), _("IFB Device"),
        _("InboundDelay"), _("InboundRate"),_("InboundLoss"),
        _("OutboundDelay"), _("OutboundRate"), _("OutboundLoss")
    };

    QGridLayout* gbox = new QGridLayout;
    for(int i = 0; i < 8; ++i) {
        gbox->addWidget(new QLabel(labels[i]), i, 0);
    }

    interfaceCombo = new ComboBox;
    for(int i = 0; i < emulator->interfaces().size(); ++i) {
        interfaceCombo->addItem(emulator->interfaces()[i].c_str());
    }
    ifbdeviceCombo = new ComboBox;
    QStringList items = { _("ifb0"), _("ifb1") };
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


void NetworkEmulatorDialog::onStartButtonToggled(const bool& on)
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
