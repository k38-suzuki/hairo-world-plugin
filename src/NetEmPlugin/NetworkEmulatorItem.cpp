/**
   \file
   \author Kenta Suzuki
*/

#include "NetworkEmulatorItem.h"
#include <cnoid/Archive>
#include <cnoid/Body>
#include <cnoid/ItemManager>
#include <cnoid/PutPropertyFunction>
#include <cnoid/SimulatorItem>
#include <cnoid/WorldItem>
#include "NetworkEmulator.h"
#include "TCAreaItem.h"
#include "gettext.h"

using namespace cnoid;
using namespace std;

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

class NetworkEmulatorItemImpl
{
public:
    NetworkEmulatorItemImpl(NetworkEmulatorItem* self);
    NetworkEmulatorItemImpl(NetworkEmulatorItem* self, const NetworkEmulatorItemImpl& org);
    NetworkEmulatorItem* self;

    NetworkEmulatorPtr emulator;
    vector<Body*> bodies;
    Selection interface;
    Selection ifbDevice;
    int prevItemID;
    SimulatorItem* simulatorItem;

    bool initializeSimulation(SimulatorItem* simulatorItem);
    void finalizeSimulation();
    void onPreDynamicsFunction();
    void doPutProperties(PutPropertyFunction& putProperty);
    bool store(Archive& archive);
    bool restore(const Archive& archive);
};

}


NetworkEmulatorItem::NetworkEmulatorItem()
{
    impl = new NetworkEmulatorItemImpl(this);
}


NetworkEmulatorItemImpl::NetworkEmulatorItemImpl(NetworkEmulatorItem* self)
    : self(self)
{
    emulator = new NetworkEmulator;
    bodies.clear();
    prevItemID = INT_MAX;
    simulatorItem = nullptr;

    interface.clear();
    for(size_t i = 0; i < emulator->interfaces().size(); ++i) {
        interface.setSymbol(i, emulator->interfaces()[i]);
    }

    ifbDevice.clear();
    ifbDevice.setSymbol(0, N_("ifb0"));
    ifbDevice.setSymbol(1, N_("ifb1"));
}


NetworkEmulatorItem::NetworkEmulatorItem(const NetworkEmulatorItem &org)
    : SubSimulatorItem(org),
      impl(new NetworkEmulatorItemImpl(this, *org.impl))
{

}


NetworkEmulatorItemImpl::NetworkEmulatorItemImpl(NetworkEmulatorItem* self, const NetworkEmulatorItemImpl& org)
    : self(self)
{
    interface = org.interface;
    ifbDevice = org.ifbDevice;
}


NetworkEmulatorItem::~NetworkEmulatorItem()
{
    delete impl;
}


void NetworkEmulatorItem::initializeClass(ExtensionManager* ext)
{
    ext->itemManager().registerClass<NetworkEmulatorItem>(N_("NetworkEmulatorItem"));
    ext->itemManager().addCreationPanel<NetworkEmulatorItem>();
}


bool NetworkEmulatorItem::initializeSimulation(SimulatorItem* simulatorItem)
{
    return impl->initializeSimulation(simulatorItem);
}


bool NetworkEmulatorItemImpl::initializeSimulation(SimulatorItem* simulatorItem)
{
    bodies.clear();
    prevItemID = INT_MAX;
    this->simulatorItem = simulatorItem;
    const vector<SimulationBody*>& simuBodies = simulatorItem->simulationBodies();
    if(simuBodies.size()) {
        for(size_t i = 0; i < simuBodies.size(); i++) {
            bodies.push_back(simuBodies[i]->body());
        }
        emulator->start(interface.which(), ifbDevice.which());
        simulatorItem->addPreDynamicsFunction([&](){ onPreDynamicsFunction(); });
    }
    return true;
}


void NetworkEmulatorItem::finalizeSimulation()
{
    impl->finalizeSimulation();
}


void NetworkEmulatorItemImpl::finalizeSimulation()
{
    emulator->stop();
}


void NetworkEmulatorItemImpl::onPreDynamicsFunction()
{
    ItemList<TCAreaItem> areaItems;
    WorldItem* worldItem = simulatorItem->findOwnerItem<WorldItem>();
    if(worldItem) {
        areaItems = worldItem->descendantItems<TCAreaItem>();
    }

    TCAreaItem* currentItem = new TCAreaItem;
    int currentItemID = INT_MAX;
    for(size_t i = 0; i < bodies.size(); ++i) {
        Body* body = bodies[i];
        if(!body->isStaticModel()) {
            Link* link = body->rootLink();
            for(size_t j = 0; j <  areaItems.size(); ++j) {
                TCAreaItem* targetItem = areaItems[j];
                if(targetItem->isCollided(link->T().translation())) {
                    currentItem = targetItem;
                    currentItemID = j;
                }
            }
        }
    }

    if(currentItemID != prevItemID) {
        static const double DELAY_MAX = 100000;
        static const double RATE_MAX = 11000000;
        static const double LOSS_MAX = 100.0;
        const int delays[] = { (int)currentItem->inboundDelay(), (int)currentItem->outboundDelay() };
        const int rates[] = { (int)currentItem->inboundRate(), (int)currentItem->outboundRate() };
        const double losses[] = { currentItem->inboundLoss(), currentItem->outboundLoss() };
        for(int i = 0; i < 2; ++i) {
            if(delays[i] >= 0 && delays[i] <= DELAY_MAX) {
                emulator->setDelay(i, delays[i]);
            }
            if(rates[i] >= 0 && rates[i] <= RATE_MAX) {
                emulator->setRate(i, rates[i]);
            }
            if(losses[i] >= 0.0 && losses[i] <= LOSS_MAX) {
                emulator->setLoss(i, losses[i]);
            }
        }
        if(checkIP(currentItem->source())) {
            emulator->setSourceIP(currentItem->source());
        }
        if(checkIP(currentItem->destination())) {
            emulator->setSourceIP(currentItem->destination());
        }
        emulator->update();
    }
    prevItemID = currentItemID;
}


Item* NetworkEmulatorItem::doDuplicate() const
{
    return new NetworkEmulatorItem(*this);
}


void NetworkEmulatorItem::doPutProperties(PutPropertyFunction& putProperty)
{
    SubSimulatorItem::doPutProperties(putProperty);
    impl->doPutProperties(putProperty);
}


void NetworkEmulatorItemImpl::doPutProperties(PutPropertyFunction& putProperty)
{
    putProperty(_("Interface"), interface,
                [&](int index){ return interface.select(index); });
    putProperty(_("IFB Device"), ifbDevice,
                [&](int index){ return ifbDevice.select(index); });
}


bool NetworkEmulatorItem::store(Archive& archive)
{
    SubSimulatorItem::store(archive);
    return impl->store(archive);
}


bool NetworkEmulatorItemImpl::store(Archive& archive)
{
    archive.write("interface", interface.selectedSymbol(), DOUBLE_QUOTED);
    archive.write("ifb_device", ifbDevice.selectedSymbol(), DOUBLE_QUOTED);
    return true;
}


bool NetworkEmulatorItem::restore(const Archive& archive)
{
    SubSimulatorItem::restore(archive);
    return impl->restore(archive);
}


bool NetworkEmulatorItemImpl::restore(const Archive& archive)
{
    interface.select(archive.get("interface", ""));
    ifbDevice.select(archive.get("ifb_device", ""));
    return true;
}