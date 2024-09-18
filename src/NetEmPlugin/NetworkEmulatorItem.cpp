/**
   @author Kenta Suzuki
*/

#include "NetworkEmulatorItem.h"
#include <cnoid/Archive>
#include <cnoid/Body>
#include <cnoid/ItemManager>
#include <cnoid/LazyCaller>
#include <cnoid/PutPropertyFunction>
#include <cnoid/SimulatorItem>
#include <cnoid/WorldItem>
#include <cnoid/MultiColliderItem>
#include "NetEm.h"
#include "gettext.h"

using namespace std;
using namespace cnoid;

namespace {

vector<string> split(const string& s, char delim)
{
    vector<string> elements;
    stringstream ss(s);
    string item;
    while(getline(ss, item, delim)) {
    if (!item.empty()) {
            elements.push_back(item);
        }
    }
    return elements;
}

bool checkIP(const string& address)
{
    bool result = false;
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
                        result = true;
                    }
                }
            }
        }
    }
    return result;
}

}

namespace cnoid {

class NetworkEmulatorItem::Impl
{
public:
    NetworkEmulatorItem* self;

    Impl(NetworkEmulatorItem* self);
    Impl(NetworkEmulatorItem* self, const Impl& org);

    NetEmPtr netem;
    vector<Body*> bodies;
    Selection interface;
    Selection ifbDevice;
    ItemList<MultiColliderItem> colliders;

    bool initializeSimulation(SimulatorItem* simulatorItem);
    void onPreDynamics();
};

}


void NetworkEmulatorItem::initializeClass(ExtensionManager* ext)
{
    ext->itemManager()
        .registerClass<NetworkEmulatorItem, SubSimulatorItem>(N_("NetworkEmulatorItem"))
        .addCreationPanel<NetworkEmulatorItem>();
}


NetworkEmulatorItem::NetworkEmulatorItem()
    : SubSimulatorItem()
{
    impl = new Impl(this);
}


NetworkEmulatorItem::Impl::Impl(NetworkEmulatorItem* self)
    : self(self)
{
    netem = new NetEm;
    bodies.clear();
    colliders.clear();

    for(size_t i = 0; i < netem->interfaces().size(); ++i) {
        interface.setSymbol(i, netem->interfaces()[i]);
    }

    ifbDevice.setSymbol(0, N_("ifb0"));
    ifbDevice.setSymbol(1, N_("ifb1"));
}


NetworkEmulatorItem::NetworkEmulatorItem(const NetworkEmulatorItem &org)
    : SubSimulatorItem(org),
      impl(new Impl(this, *org.impl))
{

}


NetworkEmulatorItem::Impl::Impl(NetworkEmulatorItem* self, const Impl& org)
    : self(self)
{
    interface = org.interface;
    ifbDevice = org.ifbDevice;
}


NetworkEmulatorItem::~NetworkEmulatorItem()
{
    delete impl;
}


bool NetworkEmulatorItem::initializeSimulation(SimulatorItem* simulatorItem)
{
    return impl->initializeSimulation(simulatorItem);
}


bool NetworkEmulatorItem::Impl::initializeSimulation(SimulatorItem* simulatorItem)
{
    bodies.clear();
    colliders.clear();

    const vector<SimulationBody*>& simBodies = simulatorItem->simulationBodies();
    for(auto& simBody : simBodies) {
        bodies.push_back(simBody->body());
    }

    WorldItem* worldItem = simulatorItem->findOwnerItem<WorldItem>();
    if(worldItem) {
        ItemList<MultiColliderItem> list = worldItem->descendantItems<MultiColliderItem>();
        for(auto& collider : list) {
            if(collider->colliderType() == MultiColliderItem::TC) {
                colliders.push_back(collider);
            }
        }
    }

    if(simBodies.size()) {
        netem->start(interface.which(), ifbDevice.which());
        simulatorItem->addPreDynamicsFunction([&](){ onPreDynamics(); });
    }
    return true;
}


void NetworkEmulatorItem::finalizeSimulation()
{
    impl->netem->stop();
}


void NetworkEmulatorItem::Impl::onPreDynamics()
{
    for(auto& body : bodies) {
        if(!body->isStaticModel()) {
            Link* link = body->rootLink();
            for(auto& collider : colliders) {
                if(collision(collider, link->T().translation())) {
                    const int delays[] = { (int)collider->inboundDelay(), (int)collider->outboundDelay() };
                    const int rates[] = { (int)collider->inboundRate(), (int)collider->outboundRate() };
                    const double losses[] = { collider->inboundLoss(), collider->outboundLoss() };
                    for(int i = 0; i < 2; ++i) {
                        if(delays[i] >= 0) {
                            netem->setDelay(i, delays[i]);
                        }
                        if(rates[i] >= 0) {
                            netem->setRate(i, rates[i]);
                        }
                        if(losses[i] >= 0.0) {
                            netem->setLoss(i, losses[i]);
                        }
                    }
                    if(checkIP(collider->source())) {
                        netem->setSourceIP(collider->source());
                    }
                    if(checkIP(collider->destination())) {
                        netem->setSourceIP(collider->destination());
                    }
                    callLater([&](){ netem->update(); });
                }
            }
        }
    }
}


Item* NetworkEmulatorItem::doCloneItem(CloneMap* cloneMap) const
{
    return new NetworkEmulatorItem(*this);
}


void NetworkEmulatorItem::doPutProperties(PutPropertyFunction& putProperty)
{
    SubSimulatorItem::doPutProperties(putProperty);
    putProperty(_("Interface"), impl->interface,
                [&](int which){ return impl->interface.select(which); });
    putProperty(_("IFB Device"), impl->ifbDevice,
                [&](int which){ return impl->ifbDevice.select(which); });
}


bool NetworkEmulatorItem::store(Archive& archive)
{
    if(!SubSimulatorItem::store(archive)) {
        return false;
    }
    archive.write("interface", impl->interface.selectedSymbol());
    archive.write("ifb_device", impl->ifbDevice.selectedSymbol());
    return true;
}


bool NetworkEmulatorItem::restore(const Archive& archive)
{
    if(!SubSimulatorItem::restore(archive)) {
        return false;
    }
    string interface;
    if(archive.read("interface", interface)) {
        impl->interface.select(interface);
    }
    if(archive.read("ifb_device", interface)) {
        impl->ifbDevice.select(interface);
    }
    return true;
}
