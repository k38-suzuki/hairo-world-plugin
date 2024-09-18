/**
   @author Kenta Suzuki
*/

#include "BeepItem.h"
#include <cnoid/Archive>
#include <cnoid/CollisionLinkPair>
#include <cnoid/Format>
#include <cnoid/ItemManager>
#include <cnoid/ItemTreeView>
#include <cnoid/LazyCaller>
#include <cnoid/PutPropertyFunction>
#include <cnoid/SimulatorItem>
#include <cnoid/WorldItem>
#include "BeepCommandItem.h"
#include "BeepEventReader.h"
#include "gettext.h"

using namespace std;
using namespace cnoid;

namespace cnoid {

class BeepItem::Impl
{
public:
    BeepItem* self;

    Impl(BeepItem* self);
    Impl(BeepItem* self, const Impl& org);

    bool initializeSimulation(SimulatorItem* simulatorItem);
    void onPostDynamics();

    typedef shared_ptr<CollisionLinkPair> CollisionLinkPairPtr;

    WorldItem* worldItem;
    string beep_event_file_path;
    vector<BeepEvent> events;
    vector<BeepCommandItem*> commandItems;
};

}


void BeepItem::initializeClass(ExtensionManager* ext)
{
    ext->itemManager()
            .registerClass<BeepItem, SubSimulatorItem>(N_("BeepItem"))
            .addCreationPanel<BeepItem>();
}


BeepItem::BeepItem()
    : SubSimulatorItem()
{
    impl = new Impl(this);
}


BeepItem::Impl::Impl(BeepItem* self)
    : self(self),
      beep_event_file_path("")
{
    worldItem = nullptr;
    events.clear();
    commandItems.clear();
}


BeepItem::BeepItem(const BeepItem& org)
    : SubSimulatorItem(org)
{
    impl = new Impl(this, *org.impl);
}


BeepItem::Impl::Impl(BeepItem* self, const Impl& org)
    : self(self)
{
    beep_event_file_path = org.beep_event_file_path;
    events = org.events;
    commandItems = org.commandItems;
}


BeepItem::~BeepItem()
{
    delete impl;
}


bool BeepItem::initializeSimulation(SimulatorItem* simulatorItem)
{
    return impl->initializeSimulation(simulatorItem);
}


bool BeepItem::Impl::initializeSimulation(SimulatorItem* simulatorItem)
{
    worldItem = simulatorItem->findOwnerItem<WorldItem>();
    events.clear();
    commandItems.clear();
    self->clearChildren();

    if(!beep_event_file_path.empty()) {
        BeepEventReader reader;
        if(reader.load(beep_event_file_path)) {
            events = reader.events();
        }

        for(auto& event: events) {
            BeepCommandItem* item = new BeepCommandItem;
            item->setName(event.name());
            item->setFrequency(event.frequency());
            item->setLength(event.length());
            item->setTemporary(true);
            self->addSubItem(item);
            commandItems.push_back(item);
        }
    }

    if(worldItem) {
        worldItem->setCollisionDetectionEnabled(true);
        simulatorItem->addPostDynamicsFunction([&](){ onPostDynamics(); });
    }

    return true;
}


void BeepItem::Impl::onPostDynamics()
{
    for(size_t i = 0; i < events.size(); ++i) {
        string link0 = events[i].link1();
        string link1 = events[i].link2();

        bool contacted = false;
        vector<CollisionLinkPairPtr>& collisions = worldItem->collisions();
        for(auto& collision : collisions) {
            LinkPtr links[2] = { collision->link(0), collision->link(1) };
            if((links[0]->name() == link0 && links[1]->name() == link1)
                    || (links[0]->name() == link1 && links[1]->name() == link0)) {
                contacted = true;
            } else if(link0 == "ALL") {
                if(links[0]->name() == link1 || links[1]->name() == link1) {
                    contacted = true;
                }
            } else if(link1 == "ALL") {
                if(links[0]->name() == link0 || links[1]->name() == link0) {
                    contacted = true;
                }
            }
            if(contacted && !events[i].isEnabled()) {
                callLater([this, i](){ commandItems[i]->execute(); });
            }
        }

        events[i].setEnabled(contacted);
    }
}


Item* BeepItem::doCloneItem(CloneMap* cloneMap) const
{
    return new BeepItem(*this);
}


void BeepItem::doPutProperties(PutPropertyFunction& putProperty)
{
    SubSimulatorItem::doPutProperties(putProperty);
    putProperty(_("Beep event file"), FilePathProperty(impl->beep_event_file_path),
                [this](const std::string& value){
                    impl->beep_event_file_path = value;
                    return true;
                });
}


bool BeepItem::store(Archive& archive)
{
    SubSimulatorItem::store(archive);
    archive.writeRelocatablePath("beep_event_file_path", impl->beep_event_file_path);
    return true;
}


bool BeepItem::restore(const Archive& archive)
{
    SubSimulatorItem::restore(archive);
    string symbol;
    if(archive.read("beep_event_file_path", symbol)) {
        symbol = archive.resolveRelocatablePath(symbol);
        if(!symbol.empty()) {
            impl->beep_event_file_path = symbol;
        }
    }
    return true;
}