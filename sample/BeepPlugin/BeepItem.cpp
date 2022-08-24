/**
   \file
   \author Kenta Suzuki
*/

#include "BeepItem.h"
#include <cnoid/Archive>
#include <cnoid/CollisionLinkPair>
#include <cnoid/ItemManager>
#include <cnoid/PutPropertyFunction>
#include <cnoid/SimulatorItem>
#include <cnoid/WorldItem>
#include <fmt/format.h>
#include "BeepView.h"
#include "BeepWidget.h"
#include "gettext.h"

using namespace cnoid;
using namespace std;

namespace cnoid {

class BeepItemImpl
{
public:
    BeepItemImpl(BeepItem* self);
    BeepItemImpl(BeepItem* self, const BeepItemImpl& org);
    BeepItem* self;

    SimulatorItem* simulatorItem;
    WorldItem* worldItem;
    BeepWidget* beepWidget;

    bool initializeSimulation(SimulatorItem* simulatorItem);
    void onPostDynamicsFunction();
    void doPutProperties(PutPropertyFunction& putProperty);
    bool store(Archive& archive);
    bool restore(const Archive& archive);
};

}


BeepItem::BeepItem()
{
    impl = new BeepItemImpl(this);
}


BeepItemImpl::BeepItemImpl(BeepItem* self)
    : self(self)
{
    simulatorItem = nullptr;
    worldItem = nullptr;
    beepWidget = nullptr;
}


BeepItem::BeepItem(const BeepItem& org)
    : SubSimulatorItem(org)
{
    impl = new BeepItemImpl(this, *org.impl);
}


BeepItemImpl::BeepItemImpl(BeepItem* self, const BeepItemImpl& org)
    : self(self)
{

}


BeepItem::~BeepItem()
{
    delete impl;
}


void BeepItem::initializeClass(ExtensionManager* ext)
{
    ext->itemManager()
            .registerClass<BeepItem, SubSimulatorItem>(N_("BeepItem"))
            .addCreationPanel<BeepItem>();
}


bool BeepItem::initializeSimulation(SimulatorItem* simulatorItem)
{
    return impl->initializeSimulation(simulatorItem);
}


bool BeepItemImpl::initializeSimulation(SimulatorItem* simulatorItem)
{
    this->simulatorItem = simulatorItem;
    worldItem = this->simulatorItem->findOwnerItem<WorldItem>();
    beepWidget = BeepView::instance()->beepWidget();

    if(worldItem) {
        worldItem->setCollisionDetectionEnabled(true);
        this->simulatorItem->addPostDynamicsFunction([&](){ onPostDynamicsFunction(); });
    }

    return true;
}


void BeepItemImpl::onPostDynamicsFunction()
{
    int currentTime = simulatorItem->currentTime() * 1000.0;
    if(beepWidget) {
        int count = beepWidget->treeWidget()->topLevelItemCount();
        for(int i = 0; i < count; ++i) {
            QTreeWidgetItem* item = beepWidget->treeWidget()->topLevelItem(i);
            string link0 = item->text(1).toStdString();
            string link1 = item->text(2).toStdString();

            bool contacted = false;
            vector<CollisionLinkPairPtr>& collisions = worldItem->collisions();
            for(int j = 0; j < collisions.size(); ++j) {
                Link* links[2] = { collisions[j]->link[0], collisions[j]->link[1] };
                if((links[0]->name() == link0 && links[1]->name() == link1)
                        || (links[0]->name() == link1 && links[1]->name() == link0)) {
                    if(currentTime % 200 == 0) {
                        contacted = true;
                    }
                } else if(link0 == "ALL") {
                    if(links[0]->name() == link1 || links[1]->name() == link1) {
                        contacted = true;
                    }
                } else if(link1 == "ALL") {
                    if(links[0]->name() == link0 || links[1]->name() == link0) {
                        contacted = true;
                    }
                }
                if(contacted) {
                    beepWidget->play(i);
                }
            }
        }
    }
}


Item* BeepItem::doDuplicate() const
{
    return new BeepItem(*this);
}


void BeepItem::doPutProperties(PutPropertyFunction& putProperty)
{
    SubSimulatorItem::doPutProperties(putProperty);
    impl->doPutProperties(putProperty);
}


void BeepItemImpl::doPutProperties(PutPropertyFunction& putProperty)
{

}


bool BeepItem::store(Archive& archive)
{
    SubSimulatorItem::store(archive);
    return impl->store(archive);
}


bool BeepItemImpl::store(Archive& archive)
{
    return true;
}


bool BeepItem::restore(const Archive& archive)
{
    SubSimulatorItem::restore(archive);
    return impl->restore(archive);
}


bool BeepItemImpl::restore(const Archive& archive)
{
    return true;
}
