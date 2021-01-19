/**
   \file
   \author Kenta Suzuki
*/

#include "TCSimulatorItem.h"
#include <cnoid/Archive>
#include <cnoid/Body>
#include <cnoid/EigenUtil>
#include <cnoid/ItemManager>
#include <cnoid/ItemTreeView>
#include <cnoid/Process>
#include <cnoid/PutPropertyFunction>
#include <cnoid/SimulatorItem>
#include <cnoid/WorldItem>
#include <fmt/format.h>
#include <stdlib.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>
#include "gettext.h"
#include "TCAreaItem.h"

#define IFR_MAX 10
#define DELAY_MAX 100000
#define RATE_MAX 11000000
#define LOSS_MAX 100.0

using namespace std;
using namespace cnoid;

namespace {

vector<string> split(const string& s, char delim)
{
    vector<string> elements;
    stringstream ss(s);
    string item;
    while (getline(ss, item, delim)) {
    if (!item.empty()) {
            elements.push_back(item);
        }
    }
    return elements;
}

}


namespace cnoid {

class TCSimulatorItemImpl
{
public:
    TCSimulatorItemImpl(TCSimulatorItem* self);
    TCSimulatorItemImpl(TCSimulatorItem* self, const TCSimulatorItemImpl& org);

    TCSimulatorItem* self;
    vector<Body*> bodies;
    ItemList<TCAreaItem> items;
    Selection interface;
    Selection ifbDevice;
    vector<string> interfaceNames;
    vector<string> ifbDeviceNames;
    string interfaceName;
    string ifbDeviceName;
    int prevItemId;
    bool init;

    bool initializeSimulation(SimulatorItem* simulatorItem);
    void finalizeSimulation();
    void doPutProperties(PutPropertyFunction& putProperty);
    bool store(Archive& archive);
    bool restore(const Archive& archive);
    void onPreDynamicsFunction();
    void onTCInitialize();
    void onTCClear();
    void onTCFinalize();
    void onTCExecute(TCAreaItem* item);
    void onCommandExecute(const string& message);
    bool onAddressCheck(const string& address) const;
};

}


TCSimulatorItem::TCSimulatorItem()
{
    impl = new TCSimulatorItemImpl(this);
}


TCSimulatorItemImpl::TCSimulatorItemImpl(TCSimulatorItem* self)
    : self(self)
{
    bodies.clear();
    items.clear();
    interface.clear();
    ifbDevice.clear();
    interfaceNames.clear();
    ifbDeviceNames.clear();
    interfaceName.clear();
    ifbDeviceName.clear();
    prevItemId = INT_MAX;
    init = false;

    struct ifreq ifr[IFR_MAX];
    struct ifconf ifc;
    int fd = socket(AF_INET, SOCK_DGRAM, 0);

    ifc.ifc_len = sizeof(ifr);
    ifc.ifc_ifcu.ifcu_buf = (char*)ifr;
    ioctl(fd, SIOCGIFCONF, &ifc);
    int nifs = ifc.ifc_len / sizeof(struct ifreq);
    int i = 0;
    interfaceNames.push_back("lo");
    interface.setSymbol(i++, "lo");
    for (int j = 0; j < nifs; j++) {
        string interfaceName = string(ifr[j].ifr_name);
        if(interfaceName != "lo") {
            interfaceNames.push_back(interfaceName);
            interface.setSymbol(i, interfaceName);
            i++;
        }
    }
    ::close(fd);

    ifbDeviceNames.push_back("ifb0");
    ifbDeviceNames.push_back("ifb1");
    ifbDevice.setSymbol(0, N_("ifb0"));
    ifbDevice.setSymbol(1, N_("ifb1"));
}


TCSimulatorItem::TCSimulatorItem(const TCSimulatorItem &org)
    : SubSimulatorItem(org),
      impl(new TCSimulatorItemImpl(this, *org.impl))
{

}


TCSimulatorItemImpl::TCSimulatorItemImpl(TCSimulatorItem* self, const TCSimulatorItemImpl& org)
    : self(self)
{
    bodies.clear();
    items.clear();
    interface = org.interface;
    ifbDevice = org.ifbDevice;
    interfaceNames = org.interfaceNames;
    ifbDeviceNames = org.ifbDeviceNames;
    interfaceName = org.interfaceName;
    ifbDeviceName = org.ifbDeviceName;
    prevItemId = org.prevItemId;
    init = org.init;
}


TCSimulatorItem::~TCSimulatorItem()
{
    delete impl;
}


void TCSimulatorItem::initializeClass(ExtensionManager* ext)
{
    ext->itemManager().registerClass<TCSimulatorItem>(N_("TCSimulatorItem"));
    ext->itemManager().addCreationPanel<TCSimulatorItem>();
}


bool TCSimulatorItem::initializeSimulation(SimulatorItem* simulatorItem)
{
    return impl->initializeSimulation(simulatorItem);
}


bool TCSimulatorItemImpl::initializeSimulation(SimulatorItem* simulatorItem)
{
    bodies.clear();
    items.clear();
    prevItemId = INT_MAX;
    init = false;
    vector<SimulationBody*> simulationBodies = simulatorItem->simulationBodies();
    if(simulationBodies.size()) {
        for(size_t i = 0; i < simulationBodies.size(); i++) {
            bodies.push_back(simulationBodies[i]->body());
        }

        simulatorItem->addPreDynamicsFunction([&](){ onPreDynamicsFunction(); });
    }

    items = ItemTreeView::instance()->checkedItems<TCAreaItem>();
    for(size_t i = 0; i < items.size(); i++) {
        TCAreaItem* item = items[i];
        item->setId(i);
    }
    onTCInitialize();
    return true;
}


void TCSimulatorItem::finalizeSimulation()
{
    impl->finalizeSimulation();
}


void TCSimulatorItemImpl::finalizeSimulation()
{
    onTCClear();
    onTCFinalize();
}


Item* TCSimulatorItem::doDuplicate() const
{
    return new TCSimulatorItem(*this);
}


void TCSimulatorItem::doPutProperties(PutPropertyFunction& putProperty)
{
    SubSimulatorItem::doPutProperties(putProperty);
    impl->doPutProperties(putProperty);
}


void TCSimulatorItemImpl::doPutProperties(PutPropertyFunction& putProperty)
{
    putProperty(_("Interface"), interface,
                [&](int index){ return interface.select(index); });
    putProperty(_("IFB Device"), ifbDevice,
                [&](int index){ return ifbDevice.select(index); });
}


bool TCSimulatorItem::store(Archive& archive)
{
    SubSimulatorItem::store(archive);
    return impl->store(archive);
}


bool TCSimulatorItemImpl::store(Archive& archive)
{
    archive.write("interface", interface.selectedSymbol(), DOUBLE_QUOTED);
    archive.write("ifbDevice", ifbDevice.selectedSymbol(), DOUBLE_QUOTED);
    return true;
}


bool TCSimulatorItem::restore(const Archive& archive)
{
    SubSimulatorItem::restore(archive);
    return impl->restore(archive);
}


bool TCSimulatorItemImpl::restore(const Archive& archive)
{
    string symbol;
    if(archive.read("interface", symbol)) {
        interface.select(symbol);
    }
    if(archive.read("ifbDevice", symbol)) {
        ifbDevice.select(symbol);
    }
    return true;
}


void TCSimulatorItemImpl::onPreDynamicsFunction()
{
    TCAreaItem* currentItem = new TCAreaItem();
    for(size_t i = 0; i < bodies.size(); i++) {
        Body* body = bodies[i];
        if(!body->isStaticModel()) {
            Link* link = body->rootLink();
            Vector3 p = link->T().translation();
            for(int j = 0; j < items.size(); j++) {
                TCAreaItem* item = items[j];
                Vector3 translation = item->translation();
                Vector3 rotation = item->rotation();
                Matrix3 s = rotFromRpy(rotation * TO_RADIAN);

                WorldItem* worldItem = item->findOwnerItem<WorldItem>();
                if(worldItem) {
                    if(item->type() == "Box") {
                        Vector3 size = item->size();
                        Vector3 minRange = translation - size / 2.0;
                        Vector3 maxRange = translation + size / 2.0;
                        if((minRange[0] <= p[0]) && (p[0] <= maxRange[0])
                                && (minRange[1] <= p[1]) && (p[1] <= maxRange[1])
                                && (minRange[2] <= p[2]) && (p[2] <= maxRange[2])
                                ) {
                            currentItem = item;
                        }
                    }
                    else if(item->type() == "Cylinder") {
                        Vector3 a = s * (Vector3(0.0, 1.0, 0.0) * item->height() / 2.0) + translation;
                        Vector3 b = s * (Vector3(0.0, 1.0, 0.0) * item->height() / 2.0 * -1.0) + translation;
                        Vector3 c = a - b;
                        Vector3 d = p - b;
                        double cd = c.dot(d);
                        if((0.0 < cd) && (cd < c.dot(c))) {
                            double r2 = d.dot(d) - d.dot(c) * d.dot(c) / c.dot(c);
                            double rp2 =  item->radius() * item->radius();
                            if(r2 < rp2) {
                                currentItem = item;
                            }
                        }
                    }
                    else if(item->type() == "Sphere") {
                        Vector3 r = translation - p;
                        if(r.norm() <= item->radius()) {
                            currentItem = item;
                        }
                    }
                }
            }
        }
    }

    if(currentItem->id() != prevItemId) {
        if(!init) {
            onTCExecute(currentItem);
            init = true;
        } else {
            onTCClear();
            onTCExecute(currentItem);
        }
    }
    prevItemId = currentItem->id();
}


void TCSimulatorItemImpl::onTCInitialize()
{
    for(int i = 0; i < interfaceNames.size(); i++) {
        if(interface.is(i)) {
            interfaceName = interfaceNames[i];
        }
    }
    for(int i = 0; i < ifbDeviceNames.size(); i++) {
        if(ifbDevice.is(i)) {
            ifbDeviceName = ifbDeviceNames[i];
        }
    }

    string message = (fmt::format("sudo modprobe ifb;"
                                  "sudo modprobe act_mirred;"
                                  "sudo ip link set dev {0} up;",
                                ifbDeviceName));
    onCommandExecute(message);
}


void TCSimulatorItemImpl::onTCClear()
{
    string message = (fmt::format("sudo tc qdisc del dev {0} ingress;"
                                  "sudo tc qdisc del dev {1} root;"
                                  "sudo tc qdisc del dev {0} root;",
                                  interfaceName, ifbDeviceName));
    onCommandExecute(message);
}


void TCSimulatorItemImpl::onTCFinalize()
{
    string message = (fmt::format("sudo ip link set dev {0} down;"
                                  "sudo rmmod ifb;",
                                  ifbDeviceName));
    onCommandExecute(message);
}


void TCSimulatorItemImpl::onTCExecute(TCAreaItem* item)
{
    double inboundDelay = item->inboundDelay();
    double inboundLoss = item->inboundLoss();
    double inboundRate = item->inboundRate();
    double outboundDelay = item->outboundDelay();
    double outboundLoss = item->outboundLoss();
    double outboundRate = item->outboundRate();

    string inboundEffects;
    string outboundEffects;

    if(inboundDelay > 0.0) {
        inboundEffects += " delay " + (fmt::format("{0:.2f}", inboundDelay)) + "ms";
    }
    if(inboundRate > 0.0) {
        inboundEffects += " rate " + (fmt::format("{0:.2f}", inboundRate)) + "kbps";
    }
    if(inboundLoss > 0.0) {
        inboundEffects += " loss " + (fmt::format("{0:.2f}", inboundLoss)) + "%";
    }

    if(outboundDelay > 0.0) {
        outboundEffects += " delay " + (fmt::format("{0:.2f}", outboundDelay)) + "ms";
    }
    if(outboundRate > 0.0) {
        outboundEffects += " rate " + (fmt::format("{0:.2f}", outboundRate)) + "kbps";
    }
    if(outboundLoss > 0.0) {
        outboundEffects += " loss " + (fmt::format("{0:.2f}", outboundLoss)) + "%";
    }

    string srcipName = item->source();
    string dstipName = item->destination();
    if(!onAddressCheck(srcipName)) {
        srcipName = "0.0.0.0/0";
    }
    if(!onAddressCheck(dstipName)) {
        dstipName = "0.0.0.0/0";
    }

    string srcMessage;
    string dstMessage;

    if((!srcipName.empty()) && (!dstipName.empty())) {
        dstMessage = (fmt::format("sudo tc qdisc add dev {0} parent 1:2 handle 20: netem limit 2000{1};"
                                  "sudo tc filter add dev {0} protocol ip parent 1: prio 2 u32 match ip src {2} match ip dst {3} flowid 1:2;",
                                ifbDeviceName, inboundEffects, dstipName, srcipName));
        srcMessage = (fmt::format("sudo tc qdisc add dev {0} parent 1:2 handle 20: netem limit 2000{1};"
                                  "sudo tc filter add dev {0} protocol ip parent 1: prio 2 u32 match ip src {2} match ip dst {3} flowid 1:2;",
                                interfaceName, outboundEffects, srcipName, dstipName));
    }

    string message = (fmt::format("sudo tc qdisc add dev {0} ingress handle ffff:;"
                                  "sudo tc filter add dev {0} parent ffff: protocol ip u32 match u32 0 0 action mirred egress redirect dev {1};"
                                  "sudo tc qdisc add dev {1} root handle 1: "
                                  "prio bands 16 priomap 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0;"
                                  "sudo tc qdisc add dev {1} parent 1:1 handle 10: netem limit 2000;"
                                  "{2}"
                                  "sudo tc qdisc add dev {0} root handle 1: "
                                  "prio bands 16 priomap 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0;"
                                  "sudo tc qdisc add dev {0} parent 1:1 handle 10: netem limit 2000;"
                                  "{3}",
                                  interfaceName, ifbDeviceName, dstMessage, srcMessage));
    onCommandExecute(message);
}


void TCSimulatorItemImpl::onCommandExecute(const string& message)
{
    vector<string> commands = split(message, ';');
    for(size_t i = 0; i < commands.size(); ++i) {
        QString command = QString::fromStdString(commands[i]);
        Process::execute(command);
    }
}


bool TCSimulatorItemImpl::onAddressCheck(const string& address) const
{
    vector<string> ip_mask = split(address, '/');
    if(ip_mask.size() == 2) {
        int mask = atoi(ip_mask[1].c_str());
        if((mask >= 0) && (mask <= 32)) {
            string ipaddress = ip_mask[0];
            vector<string> ipelements = split(ipaddress, '.');
            if(ipelements.size() == 4) {
                for(int i = 0; i < 4; i++) {
                    int element = atoi(ipelements[i].c_str());
                    if((element >= 0) && (element <= 255)) {

                    } else {
                        return false;
                    }
                }
            } else {
                return false;
            }
        } else {
            return false;
        }
    } else {
        return false;
    }
    return true;
}
