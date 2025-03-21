/**
   @author Kenta Suzuki
*/

#include "NetEm.h"
#include <cnoid/Format>
#include <QProcess>
#include <net/if.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>

using namespace std;
using namespace cnoid;

namespace cnoid {

class NetEm::Impl
{
public:

    Impl();
    ~Impl();

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

    QProcess process;

    void start();
    void write(const string& program);
    void clear();
    void update();
    void close();
};

}


NetEm::NetEm()
{
    impl = new Impl;
}


NetEm::Impl::Impl()
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

    process.start("bash", QStringList());
}


NetEm::~NetEm()
{
    delete impl;
}


NetEm::Impl::~Impl()
{
    if(!isFinalized) {
        close();
    }
    process.close();
}


vector<string>& NetEm::interfaces() const
{
    return impl->interfaces;
}


void NetEm::start(const int& interfaceID, const int& ifbdeviceID)
{
    impl->currentInterfaceID = interfaceID;
    impl->currentIfbdeviceID = ifbdeviceID;
    impl->start();
}


void NetEm::Impl::start()
{
    if(!isFinalized) {
        close();
    }
    write("sudo modprobe ifb;");
    write("sudo modprobe act_mirred;");
    write(formatC("sudo ip link set dev {0} up;",
                      ifbdevices[currentIfbdeviceID]));
    isUpdated = false;
    isFinalized = false;
}


void NetEm::update()
{
    impl->update();
}


void NetEm::Impl::update()
{
    if(!isFinalized) {
        clear();
        write(formatC("sudo tc qdisc add dev {0} ingress handle ffff:;",
                          interfaces[currentInterfaceID]));
        write(formatC("sudo tc filter add dev {0} parent ffff: protocol ip u32 match u32 0 0 action mirred egress redirect dev {1};",
                          interfaces[currentInterfaceID], ifbdevices[currentIfbdeviceID]));

        string effects[2];
        for(int i = 0 ; i < 2; ++i) {
            if(delays[i] > 0.0) {
               effects[i] +=  formatC(" delay {0:.2f}ms", delays[i]);
            }
            if(rates[i] > 0.0) {
                effects[i] +=  formatC(" rate {0:.2f}kbps", rates[i]);
            }
            if(losses[i] > 0.0) {
                effects[i] +=  formatC(" loss {0:.2f}%", losses[i]);
            }
        }

        write(formatC("sudo tc qdisc add dev {0} root handle 1: prio bands 16 priomap 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0;",
                          ifbdevices[currentIfbdeviceID]));
        write(formatC("sudo tc qdisc add dev {0} parent 1:1 handle 10: netem limit 2000;",
                          ifbdevices[currentIfbdeviceID]));
        write(formatC("sudo tc qdisc add dev {0} parent 1:2 handle 20: netem limit 2000{1};",
                          ifbdevices[currentIfbdeviceID], effects[0]));
        write(formatC("sudo tc filter add dev {0} protocol ip parent 1: prio 2 u32 match ip src {1} match ip dst {2} flowid 1:2;",
                          ifbdevices[currentIfbdeviceID], destinationIP, sourceIP));

        write(formatC("sudo tc qdisc add dev {0} root handle 1: prio bands 16 priomap 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0;",
                          interfaces[currentInterfaceID]));
        write(formatC("sudo tc qdisc add dev {0} parent 1:1 handle 10: netem limit 2000;",
                          interfaces[currentInterfaceID]));
        write(formatC("sudo tc qdisc add dev {0} parent 1:2 handle 20: netem limit 2000{1};",
                          interfaces[currentInterfaceID], effects[1]));
        write(formatC("sudo tc filter add dev {0} protocol ip parent 1: prio 2 u32 match ip src {1} match ip dst {2} flowid 1:2;",
                          interfaces[currentInterfaceID], sourceIP, destinationIP));
        isUpdated = true;
    }
}


void NetEm::stop()
{
    impl->close();
}


void NetEm::Impl::clear()
{
    if(isUpdated) {
        write(formatC("sudo tc qdisc del dev {0} ingress;",
                          interfaces[currentInterfaceID]));
        write(formatC("sudo tc qdisc del dev {0} root;",
                          ifbdevices[currentIfbdeviceID]));
        write(formatC("sudo tc qdisc del dev {0} root;",
                          interfaces[currentInterfaceID]));
        isUpdated = false;
    }
}


void NetEm::Impl::close()
{
    if(!isFinalized) {
        clear();
        write(formatC("sudo ip link set dev {0} down;",
                          ifbdevices[currentIfbdeviceID]));
        write("sudo rmmod ifb;");
        isFinalized = true;
    }
}


void NetEm::setDelay(const int& id, const double& delay)
{
    impl->delays[id] = delay;
}


void NetEm::setRate(const int& id, const double& rate)
{
    impl->rates[id] = rate;
}


void NetEm::setLoss(const int& id, const double& loss)
{
    impl->losses[id] = loss;
}


void NetEm::setSourceIP(const string& sourceIP)
{
    impl->sourceIP = sourceIP;
}


void NetEm::setDestinationIP(const string& destinationIP)
{
    impl->destinationIP = destinationIP;
}


void NetEm::Impl::write(const string& program)
{
    int ret = system(program.c_str());
}
