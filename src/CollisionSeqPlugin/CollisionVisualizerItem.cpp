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
#include <cnoid/SceneDrawables>
#include <cnoid/SimulatorItem>
#include <cnoid/StringUtil>
#include <cnoid/Tokenizer>
#include <cnoid/ValueTreeUtil>
#include "gettext.h"
#include "CollisionSensor.h"

using namespace cnoid;
using namespace std;

namespace {

string getNameListString(const vector<string>& names)
{
    string nameList;
    if(!names.empty()){
        size_t n = names.size() - 1;
        for(size_t i=0; i < n; ++i){
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
    for(auto& token : Tokenizer<CharSeparator<char>>(nameListString, CharSeparator<char>(","))){
        auto name = trimmed(token);
        if(!name.empty()){
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
    SimulatorItem* simulatorItem_;

    vector<MultiValueSeqItem*> collisionStaSeqItems;
    bool isCollisionStatesRecordingEnabled;
    int frame;
    map<SgShape*, SgMaterial*> materials;
    map<SgMaterial*, Link*> links;
    map<Link*, Vector3> colors;
    map<Link*, bool> prevs;
    DeviceList<CollisionSensor> sensors;

    void onPostDynamicsFunction();
    void initializeBody(Body* body);

    void initialize();
    void extract(SgNode* node, Link* link, Vector3 color);
    void update();
    void finalize();

    bool initializeSimulation(SimulatorItem* simulatorItem);
    void finalizeSimulation();
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
    simulatorItem_ = nullptr;
    collisionStaSeqItems.clear();
    isCollisionStatesRecordingEnabled = false;
    frame = 0;
    materials.clear();
    links.clear();
    colors.clear();
    prevs.clear();
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
    simulatorItem_ = org.simulatorItem_;
    collisionStaSeqItems = org.collisionStaSeqItems;
    isCollisionStatesRecordingEnabled = org.isCollisionStatesRecordingEnabled;
    frame = org.frame;
    materials = org.materials;
    links = org.links;
    colors = org.colors;
    prevs = org.prevs;
    sensors = org.sensors;
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
    simulatorItem_ = simulatorItem;
    collisionStaSeqItems.clear();
    frame = 0;
    materials.clear();
    links.clear();
    colors.clear();
    prevs.clear();
    sensors.clear();

    vector<SimulationBody*> simulationBodies = simulatorItem->simulationBodies();
    for(size_t i = 0; i < simulationBodies.size(); ++i) {
        Body* body = simulationBodies[i]->body();
        sensors << body->devices();
        if(!body->isStaticModel()) {
            if(bodyNames.size()) {
                for(auto& bodyName : bodyNames) {
                    if(body->name() == bodyName) {
                        initializeBody(body);
                    }
                }
            } else {
                initializeBody(body);
            }
        }
    }

    if(bodies.size()) {
        simulatorItem->addPostDynamicsFunction([&](){ onPostDynamicsFunction(); });
    }

    initialize();
    return true;
}


void CollisionVisualizerItem::finalizeSimulation()
{
    impl->finalizeSimulation();
}


void CollisionVisualizerItemImpl::finalizeSimulation()
{
    finalize();
}


void CollisionVisualizerItemImpl::onPostDynamicsFunction()
{
    for(size_t i = 0; i < bodies.size(); ++i) {
        Body* body = bodies[i];
        if(isCollisionStatesRecordingEnabled) {
            std::shared_ptr<MultiValueSeq> collisionStaSeq = collisionStaSeqItems[i]->seq();
            collisionStaSeq->setNumFrames(frame + 1);
            MultiValueSeq::Frame p = collisionStaSeq->frame(frame);
            for(int j = 0; j < body->numLinks(); ++j) {
                Link* link = body->link(j);
                auto& contacts = link->contactPoints();
                p[j] = !contacts.empty() ? 1 : 0;
            }
        }
    }
    update();
    frame++;
}


void CollisionVisualizerItemImpl::initializeBody(Body* body)
{
    bodies.push_back(body);
    if(isCollisionStatesRecordingEnabled) {
        MultiValueSeqItem* collisionStaSeqItem = new MultiValueSeqItem();
        string name = "Collision States - " + body->name();
        collisionStaSeqItem->setName(name);
        self->addSubItem(collisionStaSeqItem);
        collisionStaSeqItems.push_back(collisionStaSeqItem);

        int numParts = body->numLinks();
        std::shared_ptr<MultiValueSeq> collisionStaSeq = collisionStaSeqItem->seq();
        collisionStaSeq->setSeqContentName("CollisionStaSeq");
        collisionStaSeq->setNumParts(numParts);
        collisionStaSeq->setDimension(0, numParts, 1);
        collisionStaSeq->setFrameRate(1.0 / simulatorItem_->worldTimeStep());

        for(int j = 0; j < body->numLinks(); ++j) {
            Link* link = body->link(j);
            link->mergeSensingMode(Link::LinkContactState);
        }
    }
}


void CollisionVisualizerItemImpl::initialize()
{
    for(size_t i = 0; i < sensors.size(); ++i) {
        CollisionSensor* sensor = sensors[i];
        if(sensor->on()) {
            Link* link = sensor->link();
            link->mergeSensingMode(Link::LinkContactState);
            Vector3 color = sensor->color();
            SgGroup* group = link->shape();
            if(group) {
                extract(group, link, color);
            }
        }
    }
}


void CollisionVisualizerItemImpl::extract(SgNode* node, Link* link, Vector3 color)
{
    if(node->isGroupNode()) {
        SgGroup* group = dynamic_cast<SgGroup*>(node);
        if(group) {
            for(int i = 0; i < group->numChildren(); ++i) {
                SgNode* n = group->child(i);
                extract(n, link, color);
            }
        }
    } else {
        SgShape* shape = dynamic_cast<SgShape*>(node);
        if(shape) {
            SgMaterial* material = new SgMaterial(*shape->material());
            if(material && link) {
                materials[shape] = material;
                links[material] = link;
                colors[link] = color;
            }
        }
    }
}


void CollisionVisualizerItemImpl::update()
{
    bool changed = false;
    for(auto itr = materials.begin(); itr != materials.end(); ++itr) {
        SgShape* shape = itr->first;
        SgMaterial* material = itr->second;
        if(shape && material) {
            Link* link = links[material];
            SgMaterial* material2 = new SgMaterial(*material);
            if(link && material2) {
                Vector3 color = colors[link];
                auto& contacts = link->contactPoints();
                if(!contacts.empty()) {
                    changed = true;
                    material2->setDiffuseColor(color);
                }
                if(changed != prevs[link]) {
                    shape->setMaterial(material2);
                    shape->notifyUpdate();
                }
            }
            prevs[link] = changed;
        }
    }
}


void CollisionVisualizerItemImpl::finalize()
{
    for(auto itr = materials.begin(); itr != materials.end(); ++itr) {
        SgShape* shape = itr->first;
        SgMaterial* material = itr->second;
        if(shape && material) {
            shape->setMaterial(material);
            shape->notifyUpdate();
        }
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
    putProperty(_("Record collision states"), isCollisionStatesRecordingEnabled, changeProperty(isCollisionStatesRecordingEnabled));
}


bool CollisionVisualizerItem::store(Archive& archive)
{
    SubSimulatorItem::store(archive);
    return impl->store(archive);
}


bool CollisionVisualizerItemImpl::store(Archive& archive)
{
    writeElements(archive, "targetBodies", bodyNames, true);
    archive.write("recordCollisionStates", isCollisionStatesRecordingEnabled);
    return true;
}


bool CollisionVisualizerItem::restore(const Archive& archive)
{
    SubSimulatorItem::restore(archive);
    return impl->restore(archive);
}


bool CollisionVisualizerItemImpl::restore(const Archive& archive)
{
    readElements(archive, "targetBodies", bodyNames);
    bodyNameListString = getNameListString(bodyNames);
    archive.read("recordCollisionStates", isCollisionStatesRecordingEnabled);
    return true;
}
