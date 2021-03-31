/**
   \file
   \author Kenta Suzuki
*/

#include "OperationReviewCollectorItem.h"
#include <cnoid/Archive>
#include <cnoid/ItemManager>
#include <cnoid/Joystick>
#include <cnoid/MultiValueSeq>
#include <cnoid/MultiValueSeqItem>
#include <cnoid/PutPropertyFunction>
#include <cnoid/SimulatorItem>
#include "gettext.h"

using namespace std;
using namespace cnoid;

namespace cnoid {

class OperationReviewCollectorItemImpl
{
public:
    OperationReviewCollectorItemImpl(OperationReviewCollectorItem* self);
    OperationReviewCollectorItemImpl(OperationReviewCollectorItem* self, const OperationReviewCollectorItemImpl& org);
    OperationReviewCollectorItem* self;

    vector<Joystick*> joysticks;
    vector<MultiValueSeqItem*> joystickStaSeqItems;
    vector<MultiValueSeqItem*> collisionStaSeqItems;
    int frame;
    bool isJoystickStatesRecordingEnabled;
    bool isCollisionStatesRecordingEnabled;
    int id;
    vector<Body*> bodies;

    bool initializeSimulation(SimulatorItem* simulatorItem);
    void onPostDynamicsFunction();
    void doPutProperties(PutPropertyFunction& putProperty);
    bool store(Archive& archive);
    bool restore(const Archive& archive);
};

}


OperationReviewCollectorItem::OperationReviewCollectorItem()
{
    impl = new OperationReviewCollectorItemImpl(this);
}


OperationReviewCollectorItemImpl::OperationReviewCollectorItemImpl(OperationReviewCollectorItem* self)
    : self(self)
{
    joysticks.clear();
    joystickStaSeqItems.clear();
    collisionStaSeqItems.clear();
    frame = 0;
    isJoystickStatesRecordingEnabled = false;
    isCollisionStatesRecordingEnabled = false;
    id = 0;
    bodies.clear();
}


OperationReviewCollectorItem::OperationReviewCollectorItem(const OperationReviewCollectorItem& org)
    : SubSimulatorItem(org),
      impl(new OperationReviewCollectorItemImpl(this, *org.impl))

{

}


OperationReviewCollectorItemImpl::OperationReviewCollectorItemImpl(OperationReviewCollectorItem* self, const OperationReviewCollectorItemImpl& org)
    : self(self)
{
    joysticks = org.joysticks;
    joystickStaSeqItems = org.joystickStaSeqItems;
    collisionStaSeqItems = org.collisionStaSeqItems;
    frame = org.frame;
    isJoystickStatesRecordingEnabled = org.isJoystickStatesRecordingEnabled;
    isCollisionStatesRecordingEnabled = org.isCollisionStatesRecordingEnabled;
    id = org.id;
    bodies = org.bodies;
}


OperationReviewCollectorItem::~OperationReviewCollectorItem()
{
    delete impl;
}


void OperationReviewCollectorItem::initializeClass(ExtensionManager* ext)
{
    ext->itemManager().registerClass<OperationReviewCollectorItem>(N_("OperationReviewCollectorItem"));
    ext->itemManager().addCreationPanel<OperationReviewCollectorItem>();
}


bool OperationReviewCollectorItem::initializeSimulation(SimulatorItem* simulatorItem)
{
    return impl->initializeSimulation(simulatorItem);
}


bool OperationReviewCollectorItemImpl::initializeSimulation(SimulatorItem* simulatorItem)
{
    joysticks.clear();
    joystickStaSeqItems.clear();
    collisionStaSeqItems.clear();
    frame = 0;
    bodies.clear();

    if(isJoystickStatesRecordingEnabled) {
        for(int i = 0; i < 1; ++i) {
            string device = "/dev/input/js" + to_string(i);
            Joystick* joystick = joystick = new Joystick(device.c_str());
            if(joystick) {
                joysticks.push_back(joystick);
            }
        }

        for(size_t i = 0; i < joysticks.size(); ++i) {
            MultiValueSeqItem* joystickStaSeqItem = new MultiValueSeqItem();
            string name = "Joystick States - /dev/input/js" + to_string(i);
            joystickStaSeqItem->setName(name);
            self->addSubItem(joystickStaSeqItem);
            joystickStaSeqItems.push_back(joystickStaSeqItem);

            Joystick* joystick = joysticks[i];
            int numParts = joystick->numAxes() + joystick->numButtons();
            std::shared_ptr<MultiValueSeq> joystickStaSeq = joystickStaSeqItem->seq();
            joystickStaSeq->setSeqContentName("JoystickStaSeq");
            joystickStaSeq->setNumParts(numParts);
            joystickStaSeq->setDimension(0, numParts, 1);
            joystickStaSeq->setFrameRate(1.0 / simulatorItem->worldTimeStep());
        }
    }

    if(isCollisionStatesRecordingEnabled) {
        vector<SimulationBody*> simulationBodies = simulatorItem->simulationBodies();
        for(size_t i = 0; i < simulationBodies.size(); ++i) {
            Body* body = simulationBodies[i]->body();
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
            collisionStaSeq->setFrameRate(1.0 / simulatorItem->worldTimeStep());

            for(int j = 0; j < body->numLinks(); ++j) {
                Link* link = body->link(j);
                link->mergeSensingMode(Link::LinkContactState);
            }
            bodies.push_back(body);
        }
    }

    simulatorItem->addPostDynamicsFunction([&](){ onPostDynamicsFunction(); });
    return true;
}


void OperationReviewCollectorItemImpl::onPostDynamicsFunction()
{
    for(size_t i = 0; i < joysticks.size(); ++i) {
        Joystick* joystick = joysticks[i];
        joystick->readCurrentState();
        std::shared_ptr<MultiValueSeq> joystickStaSeq = joystickStaSeqItems[i]->seq();
        joystickStaSeq->setNumFrames(frame + 1);
        MultiValueSeq::Frame p = joystickStaSeq->frame(frame);
        for(int i = 0; i < joystick->numAxes(); ++i) {
            p[i] = joystick->getPosition(i);
        }
        for(int i = 0; i < joystick->numButtons(); ++i) {
            p[i + joystick->numAxes()] = joystick->getButtonState(i) ? 1 : 0;
        }
    }

    for(size_t i = 0; i < bodies.size(); ++i) {
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

    frame++;
}


Item* OperationReviewCollectorItem::doDuplicate() const
{
    return new OperationReviewCollectorItem(*this);
}


void OperationReviewCollectorItem::doPutProperties(PutPropertyFunction& putProperty)
{
    SubSimulatorItem::doPutProperties(putProperty);
    impl->doPutProperties(putProperty);
}


void OperationReviewCollectorItemImpl::doPutProperties(PutPropertyFunction& putProperty)
{
    putProperty(_("Record joystick states"), isJoystickStatesRecordingEnabled, changeProperty(isJoystickStatesRecordingEnabled));
    putProperty(_("Record collision states"), isCollisionStatesRecordingEnabled, changeProperty(isCollisionStatesRecordingEnabled));
}


bool OperationReviewCollectorItem::store(Archive& archive)
{
    SubSimulatorItem::store(archive);
    return impl->store(archive);
}


bool OperationReviewCollectorItemImpl::store(Archive& archive)
{
    archive.write("recordJoystickStates", isJoystickStatesRecordingEnabled);
    archive.write("recordCollisionStates", isCollisionStatesRecordingEnabled);
    return true;
}


bool OperationReviewCollectorItem::restore(const Archive& archive)
{
    SubSimulatorItem::restore(archive);
    return impl->restore(archive);
}


bool OperationReviewCollectorItemImpl::restore(const Archive& archive)
{
    archive.read("recordJoystickStates", isJoystickStatesRecordingEnabled);
    archive.read("recordCollisionStates", isCollisionStatesRecordingEnabled);
    return true;
}
