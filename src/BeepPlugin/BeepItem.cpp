/**
   @author Kenta Suzuki
*/

#include "BeepItem.h"
#include <cnoid/Archive>
#include <cnoid/CollisionLinkPair>
#include <cnoid/ItemManager>
#include <cnoid/LazyCaller>
#include <cnoid/PutPropertyFunction>
#include <cnoid/SimulatorItem>
#include <cnoid/WorldItem>
#include <fmt/format.h>
#include "BeepView.h"
#include "gettext.h"

using namespace std;
using namespace cnoid;

namespace {

struct PairInfo {
    string link0;
    string link1;
    bool isContacted;
};

}

namespace cnoid {

class BeepItem::Impl
{
public:
    BeepItem* self;

    Impl(BeepItem* self);
    Impl(BeepItem* self, const Impl& org);

    typedef shared_ptr<CollisionLinkPair> CollisionLinkPairPtr;

    SimulatorItem* simulatorItem;
    WorldItem* worldItem;
    vector<PairInfo> pairs;
    bool isPlayed;

    bool initializeSimulation(SimulatorItem* simulatorItem);
    void onPostDynamics();
};

}


void BeepItem::initializeClass(ExtensionManager* ext)
{
    ext->itemManager()
            .registerClass<BeepItem, SubSimulatorItem>(N_("BeepItem"))
            .addCreationPanel<BeepItem>();
}


BeepItem::BeepItem()
{
    impl = new Impl(this);
}


BeepItem::Impl::Impl(BeepItem* self)
    : self(self)
{
    simulatorItem = nullptr;
    worldItem = nullptr;
    pairs.clear();
    isPlayed = false;
}


BeepItem::BeepItem(const BeepItem& org)
    : SubSimulatorItem(org)
{
    impl = new Impl(this, *org.impl);
}


BeepItem::Impl::Impl(BeepItem* self, const Impl& org)
    : self(self)
{

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
    this->simulatorItem = simulatorItem;
    worldItem = this->simulatorItem->findOwnerItem<WorldItem>();
    BeepView* bv = BeepView::instance();
    pairs.clear();
    isPlayed = false;

    if(bv) {
        int numItems = bv->treeWidget()->topLevelItemCount();
        for(int i = 0; i < numItems; ++i) {
            QTreeWidgetItem* item = bv->treeWidget()->topLevelItem(i);
            string link0 = item->text(1).toStdString();
            string link1 = item->text(2).toStdString();
            PairInfo info = { link0, link1, false };
            pairs.push_back(info);
        }

        if(worldItem) {
            worldItem->setCollisionDetectionEnabled(true);
            this->simulatorItem->addPostDynamicsFunction([&](){ onPostDynamics(); });
        }
    }

    return true;
}


void BeepItem::Impl::onPostDynamics()
{
    double currentTime = simulatorItem->currentTime();
    static double startTime = 0.0;

    int playID = 999;
    for(size_t i = 0; i < pairs.size(); ++i) {
        PairInfo& info = pairs[i];
        string link0 = info.link0;
        string link1 = info.link1;

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
            if(contacted && !info.isContacted) {
                playID = i;
            }
        }

        info.isContacted = contacted;
    }

    if(playID != 999 && !isPlayed) {
        startTime = currentTime;
    }

    if(currentTime < startTime + 0.2) {
        callLater([=](){ BeepView::instance()->play(playID); });
        isPlayed = true;
    } else {
        isPlayed = false;
    }
}


Item* BeepItem::doCloneItem(CloneMap* cloneMap) const
{
    return new BeepItem(*this);
}


void BeepItem::doPutProperties(PutPropertyFunction& putProperty)
{
    SubSimulatorItem::doPutProperties(putProperty);
}


bool BeepItem::store(Archive& archive)
{
    SubSimulatorItem::store(archive);
    return true;
}


bool BeepItem::restore(const Archive& archive)
{
    SubSimulatorItem::restore(archive);
    return true;
}