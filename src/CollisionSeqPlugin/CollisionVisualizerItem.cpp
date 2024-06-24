/**
   @author Kenta Suzuki
*/

#include "CollisionVisualizerItem.h"
#include <cnoid/Archive>
#include <cnoid/Body>
#include <cnoid/DeviceList>
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

using namespace std;
using namespace cnoid;

namespace {

string getNameListString(const vector<string>& names)
{
    string nameList;
    if(!names.empty()) {
        size_t n = names.size() - 1;
        for(size_t i = 0; i < n; ++i) {
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

class CollisionVisualizerItem::Impl
{
public:
    CollisionVisualizerItem* self;

    Impl(CollisionVisualizerItem* self);
    Impl(CollisionVisualizerItem* self, const Impl& org);

    vector<Body*> bodies;
    vector<string> bodyNames;
    string bodyNameListString;
    SimulatorItem* simulatorItem;

    vector<MultiValueSeqItem*> collisionStateSeqItems;
    DeviceList<CollisionSensor> sensors;

    bool initializeSimulation(SimulatorItem* simulatorItem);
    void extract(SgNode* node, Link* link, Vector3 color);
    void onPostDynamics();
};

}


CollisionVisualizerItem::CollisionVisualizerItem()
{
    impl = new Impl(this);
}


CollisionVisualizerItem::Impl::Impl(CollisionVisualizerItem* self)
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
      impl(new Impl(this, *org.impl))
{

}


CollisionVisualizerItem::Impl::Impl(CollisionVisualizerItem* self, const Impl& org)
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
    ext->itemManager()
        .registerClass<CollisionVisualizerItem, SubSimulatorItem>(N_("CollisionVisualizerItem"))
        .addCreationPanel<CollisionVisualizerItem>();
}


bool CollisionVisualizerItem::initializeSimulation(SimulatorItem* simulatorItem)
{
    impl->initializeSimulation(simulatorItem);
    return true;
}


bool CollisionVisualizerItem::Impl::initializeSimulation(SimulatorItem* simulatorItem)
{
    bodies.clear();
    this->simulatorItem = simulatorItem;
    collisionStateSeqItems.clear();
    sensors.clear();

    std::set<string> bodyNameSet;
    for(auto& bodyName : bodyNames) {
        bodyNameSet.insert(bodyName);
    }

    const vector<SimulationBody*>& simBodies = simulatorItem->simulationBodies();
    for(auto& simBody : simBodies) {
        Body* body = simBody->body();
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
                shared_ptr<MultiValueSeq> log = collisionStateSeqItem->seq();
                log->setNumFrames(0);
                log->setNumParts(numParts);
                log->setFrameRate(1.0 / simulatorItem->worldTimeStep());
                log->setOffsetTime(0.0);

                for(int j = 0; j < body->numLinks(); ++j) {
                    Link* link = body->link(j);
                    link->mergeSensingMode(Link::LinkContactState);
                }
            }
        }
    }

    for(auto& sensor : sensors) {
        Link* link = sensor->link();
        link->mergeSensingMode(Link::LinkContactState);
    }

    if(bodies.size()) {
        this->simulatorItem->addPostDynamicsFunction([&](){ onPostDynamics(); });
    }

    return true;
}


void CollisionVisualizerItem::Impl::onPostDynamics()
{
    int currentFrame = simulatorItem->currentFrame();
    for(size_t i = 0; i < bodies.size(); ++i) {
        Body* body = bodies[i];
        shared_ptr<MultiValueSeq> log = collisionStateSeqItems[i]->seq();
        auto frame = log->appendFrame();
        for(int j = 0; j < body->numLinks(); ++j) {
            Link* link = body->link(j);
            auto& contacts = link->contactPoints();
            frame[j] = !contacts.empty() ? 1 : 0;
        }
    }

    for(auto& sensor : sensors) {
        Link* link = sensor->link();
        auto& contacts = link->contactPoints();
        sensor->on(!contacts.empty());
        sensor->notifyStateChange();
    }
}


Item* CollisionVisualizerItem::doCloneItem(CloneMap* cloneMap) const
{
    return new CollisionVisualizerItem(*this);
}


void CollisionVisualizerItem::doPutProperties(PutPropertyFunction& putProperty)
{
    SubSimulatorItem::doPutProperties(putProperty);
    putProperty(_("Target bodies"), impl->bodyNameListString,
                [&](const string& names){ return updateNames(names, impl->bodyNameListString, impl->bodyNames); });
}


bool CollisionVisualizerItem::store(Archive& archive)
{
    SubSimulatorItem::store(archive);
    writeElements(archive, "target_bodies", impl->bodyNames, true);
    return true;
}


bool CollisionVisualizerItem::restore(const Archive& archive)
{
    SubSimulatorItem::restore(archive);
    readElements(archive, "target_bodies", impl->bodyNames);
    impl->bodyNameListString = getNameListString(impl->bodyNames);
    return true;
}
