/**
   \file
   \author Kenta Suzuki
*/

#include "CollisionVisualizerItem.h"
#include <cnoid/Archive>
#include <cnoid/ItemManager>
#include <cnoid/MultiValueSeq>
#include <cnoid/MultiValueSeqItem>
#include <cnoid/PutPropertyFunction>
#include <cnoid/SimulatorItem>
#include <cnoid/StringUtil>
#include <cnoid/Tokenizer>
#include <cnoid/ValueTreeUtil>
#include <QDateTime>
#include <set>
#include "CollisionSensor.h"
#include "gettext.h"

using namespace cnoid;
using namespace std;

namespace {

string getNameListString(const vector<string>& names)
{
    string nameList;
    if(!names.empty()) {
        size_t n = names.size() - 1;
        for(size_t i=0; i < n; ++i) {
            nameList += names[i];
            nameList += ", ";
        }
        nameList += names.back();
    }
    return nameList;
}

bool updateNames(const string& nameListString, string& out_newNameListString, vector<string>& out_names)
{
    out_names.clear();
    for(auto& token : Tokenizer<CharSeparator<char>>(nameListString, CharSeparator<char>(","))) {
        auto name = trimmed(token);
        if(!name.empty()) {
            out_names.push_back(name);
        }
    }
    out_newNameListString = nameListString;
    return true;
}

}


namespace cnoid {

class CollisionVisualizerItemImpl
{
public:
    CollisionVisualizerItemImpl(CollisionVisualizerItem* self);
    CollisionVisualizerItemImpl(CollisionVisualizerItem* self, const CollisionVisualizerItemImpl& org);
    CollisionVisualizerItem* self;

    vector<Body*> bodies;
    vector<string> bodyNames;
    string bodyNameListString;
    SimulatorItem* simulatorItem;

    vector<MultiValueSeqItem*> collisionStateSeqItems;
    DeviceList<CollisionSensor> sensors;

    bool initializeSimulation(SimulatorItem* simulatorItem);
    void extract(SgNode* node, Link* link, Vector3 color);
    void onPostDynamicsFunction();
    void doPutProperties(PutPropertyFunction& putProperty);
    bool store(Archive& archive);
    bool restore(const Archive& archive);
};

}


CollisionVisualizerItem::CollisionVisualizerItem()
{
    impl = new CollisionVisualizerItemImpl(this);
}


CollisionVisualizerItemImpl::CollisionVisualizerItemImpl(CollisionVisualizerItem* self)
    : self(self)
{
    bodies.clear();
    bodyNameListString.clear();
    simulatorItem = nullptr;
    collisionStateSeqItems.clear();
    sensors.clear();
}


CollisionVisualizerItem::CollisionVisualizerItem(const CollisionVisualizerItem& org)
    : SubSimulatorItem(org),
      impl(new CollisionVisualizerItemImpl(this, *org.impl))
{

}


CollisionVisualizerItemImpl::CollisionVisualizerItemImpl(CollisionVisualizerItem* self, const CollisionVisualizerItemImpl& org)
    : self(self),
      bodyNames(org.bodyNames)
{
    bodies = org.bodies;
    bodyNameListString = getNameListString(bodyNames);
}


CollisionVisualizerItem::~CollisionVisualizerItem()
{
    delete impl;
}


void CollisionVisualizerItem::initializeClass(ExtensionManager* ext)
{
    ext->itemManager().registerClass<CollisionVisualizerItem>(N_("CollisionVisualizerItem"));
    ext->itemManager().addCreationPanel<CollisionVisualizerItem>();
}


bool CollisionVisualizerItem::initializeSimulation(SimulatorItem* simulatorItem)
{
    impl->initializeSimulation(simulatorItem);
    return true;
}


bool CollisionVisualizerItemImpl::initializeSimulation(SimulatorItem* simulatorItem)
{
    bodies.clear();
    this->simulatorItem = simulatorItem;
    collisionStateSeqItems.clear();
    sensors.clear();

    std::set<string> bodyNameSet;
    for(size_t i = 0; i < bodyNames.size(); ++i) {
        bodyNameSet.insert(bodyNames[i]);
    }

    const vector<SimulationBody*>& simBodies = simulatorItem->simulationBodies();
    for(size_t i = 0; i < simBodies.size(); ++i) {
        Body* body = simBodies[i]->body();
        if(bodyNameSet.empty() || bodyNameSet.find(body->name()) != bodyNameSet.end()) {
            sensors << body->devices();
            if(!body->isStaticModel()) {
                bodies.push_back(body);
                MultiValueSeqItem* collisionStateSeqItem = new MultiValueSeqItem;
                QDateTime loggingStartTime = QDateTime::currentDateTime();
                string suffix = loggingStartTime.toString("yyyy-MM-dd-hh-mm-ss").toStdString();
                string name = suffix + " - " + body->name();
                collisionStateSeqItem->setName(name);
                self->addSubItem(collisionStateSeqItem);
                collisionStateSeqItems.push_back(collisionStateSeqItem);

                int numParts = body->numLinks();
                shared_ptr<MultiValueSeq> collisionStateSeq = collisionStateSeqItem->seq();
                collisionStateSeq->setSeqContentName("CollisionStateSeq");
                collisionStateSeq->setFrameRate(1.0 / simulatorItem->worldTimeStep());
                collisionStateSeq->setDimension(0, numParts, false);
                collisionStateSeq->setOffsetTime(0.0);

                for(int j = 0; j < body->numLinks(); ++j) {
                    Link* link = body->link(j);
                    link->mergeSensingMode(Link::LinkContactState);
                }
            }
        }
    }

    for(size_t i = 0; i < sensors.size(); ++i) {
        CollisionSensor* sensor = sensors[i];
        Link* link = sensor->link();
        link->mergeSensingMode(Link::LinkContactState);
    }

    if(bodies.size()) {
        this->simulatorItem->addPostDynamicsFunction([&](){ onPostDynamicsFunction(); });
    }

    return true;
}


void CollisionVisualizerItemImpl::onPostDynamicsFunction()
{
    int currentFrame = simulatorItem->currentFrame();
    for(size_t i = 0; i < bodies.size(); ++i) {
        Body* body = bodies[i];
        shared_ptr<MultiValueSeq> collisionStateSeq = collisionStateSeqItems[i]->seq();
        collisionStateSeq->setNumFrames(currentFrame);
        MultiValueSeq::Frame p = collisionStateSeq->frame(currentFrame - 1);
        for(int j = 0; j < body->numLinks(); ++j) {
            Link* link = body->link(j);
            auto& contacts = link->contactPoints();
            p[j] = !contacts.empty() ? 1 : 0;
        }
    }

    for(size_t i = 0; i < sensors.size(); ++i) {
        CollisionSensor* sensor = sensors[i];
        Link* link = sensor->link();
        auto& contacts = link->contactPoints();
        sensor->on(!contacts.empty());
        sensor->notifyStateChange();
    }
}


Item* CollisionVisualizerItem::doDuplicate() const
{
    return new CollisionVisualizerItem(*this);
}


void CollisionVisualizerItem::doPutProperties(PutPropertyFunction& putProperty)
{
    SubSimulatorItem::doPutProperties(putProperty);
    impl->doPutProperties(putProperty);
}


void CollisionVisualizerItemImpl::doPutProperties(PutPropertyFunction& putProperty)
{
    putProperty(_("Target bodies"), bodyNameListString,
                [&](const string& names){ return updateNames(names, bodyNameListString, bodyNames); });
}


bool CollisionVisualizerItem::store(Archive& archive)
{
    SubSimulatorItem::store(archive);
    return impl->store(archive);
}


bool CollisionVisualizerItemImpl::store(Archive& archive)
{
    writeElements(archive, "target_bodies", bodyNames, true);
    return true;
}


bool CollisionVisualizerItem::restore(const Archive& archive)
{
    SubSimulatorItem::restore(archive);
    return impl->restore(archive);
}


bool CollisionVisualizerItemImpl::restore(const Archive& archive)
{
    readElements(archive, "target_bodies", bodyNames);
    bodyNameListString = getNameListString(bodyNames);
    return true;
}
