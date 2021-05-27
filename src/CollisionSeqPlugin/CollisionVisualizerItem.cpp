/**
   \file
   \author Kenta Suzuki
*/

#include "CollisionVisualizerItem.h"
#include <cnoid/Archive>
#include <cnoid/ItemManager>
#include <cnoid/MeshExtractor>
#include <cnoid/MultiValueSeq>
#include <cnoid/MultiValueSeqItem>
#include <cnoid/PutPropertyFunction>
#include <cnoid/SceneDrawables>
#include <cnoid/SimulatorItem>
#include <cnoid/StringUtil>
#include <cnoid/Tokenizer>
#include <cnoid/ValueTreeUtil>
#include "gettext.h"

using namespace cnoid;
using namespace std;

namespace {

string getKey(Link* link)
{
    return link->body()->name() + " " + link->name();
}

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
    MeshExtractor extractor;
    map<string, SgMaterial*> materialMap;
    vector<string> bodyNames;
    string bodyNameListString;
    SimulatorItem* simulatorItem_;

    vector<MultiValueSeqItem*> collisionStaSeqItems;
    bool isCollisionStatesRecordingEnabled;
    int frame;

    void onPostDynamicsFunction();
    void initializeBody(Body* body);
    void extractMaterial(Link* link);
    void injectMaterial(Link* link);
    void initilizeMaterial(Body* body);
    void finalizeMaterial(Link* link);

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
    materialMap.clear();
    bodyNameListString.clear();
    simulatorItem_ = nullptr;
    collisionStaSeqItems.clear();
    isCollisionStatesRecordingEnabled = false;
    frame = 0;
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
    materialMap = org.materialMap;
    bodyNameListString = getNameListString(bodyNames);
    simulatorItem_ = org.simulatorItem_;
    collisionStaSeqItems = org.collisionStaSeqItems;
    isCollisionStatesRecordingEnabled = org.isCollisionStatesRecordingEnabled;
    frame = org.frame;
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
    materialMap.clear();
    simulatorItem_ = simulatorItem;
    collisionStaSeqItems.clear();
    frame = 0;

    vector<SimulationBody*> simulationBodies = simulatorItem->simulationBodies();
    for(size_t i = 0; i < simulationBodies.size(); ++i) {
        Body* body = simulationBodies[i]->body();
        if(!body->isStaticModel()) {
            if(bodyNames.size()) {
                for(auto& bodyName : bodyNames) {
                    if(body->name() == bodyName) {
                        initializeBody(body);
                        initilizeMaterial(body);
                    }
                }
            } else {
                initializeBody(body);
                initilizeMaterial(body);
            }
        }
    }

    if(bodies.size()) {
        simulatorItem->addPostDynamicsFunction([&](){ onPostDynamicsFunction(); });
    }
    return true;
}


void CollisionVisualizerItem::finalizeSimulation()
{
    impl->finalizeSimulation();
}


void CollisionVisualizerItemImpl::finalizeSimulation()
{
    for(size_t i = 0; i < bodies.size(); ++i) {
        Body* body = bodies[i];
        for(int j = 0; j < body->numLinks(); ++j) {
            Link* link = body->link(j);
            link->mergeSensingMode(Link::LinkContactState);
            SgGroup* group = link->collisionShape();
            if(!extractor.extract(group, [&, link](){ finalizeMaterial(link); })) {

            }
        }
    }
}


void CollisionVisualizerItemImpl::onPostDynamicsFunction()
{
    for(size_t i = 0; i < bodies.size(); ++i) {
        Body* body = bodies[i];
        for(int j = 0; j < body->numLinks(); ++j) {
            Link* link = body->link(j);
            SgGroup* group = link->collisionShape();
            if(!extractor.extract(group, [&, link](){ injectMaterial(link); })) {

            }

//            auto& contacts = link->contactPoints();
//            if(!contacts.empty()) {
//                for(auto& contact : contacts) {
//                    const Vector3& p = contact.position();
//                }
//            }
        }

        if(isCollisionStatesRecordingEnabled) {
            Body* body = bodies[i];
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

    frame++;
}


void CollisionVisualizerItemImpl::initializeBody(Body* body)
{
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


void CollisionVisualizerItemImpl::extractMaterial(Link* link)
{
    SgShape* shape = extractor.currentShape();
    string key = getKey(link);
    SgMaterial* material = shape->material();
    if(!material) {
        material = new SgMaterial();
    } else {
        material = new SgMaterial(*shape->material());
    }
    materialMap[key] = material;
}


void CollisionVisualizerItemImpl::injectMaterial(Link* link)
{
    SgShape* shape = extractor.currentShape();
    string key = getKey(link);
    SgMaterial* material = materialMap[key];
    if(!material) {
        material = new SgMaterial();
    } else {
        material = new SgMaterial(*materialMap[key]);
    }

    auto& contacts = link->contactPoints();
    if(!contacts.empty()) {
        Vector3f color(1.0, 0.0, 0.0);
        material->setDiffuseColor(color);
    }
    shape->setMaterial(material);
}


void CollisionVisualizerItemImpl::initilizeMaterial(Body* body)
{
    for(int j = 0; j < body->numLinks(); ++j) {
        Link* link = body->link(j);
        link->mergeSensingMode(Link::LinkContactState);
        SgGroup* group = link->collisionShape();
        if(!extractor.extract(group, [&, link](){ extractMaterial(link); })) {

        }
    }
    bodies.push_back(body);
}


void CollisionVisualizerItemImpl::finalizeMaterial(Link* link)
{
    SgShape* shape = extractor.currentShape();
    string key = getKey(link);
    SgMaterial* material = materialMap[key];
    if(!material) {
        material = new SgMaterial();
    } else {
        material = new SgMaterial(*materialMap[key]);
    }
    shape->setMaterial(material);
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
