/**
   \file
   \author Kenta Suzuki
*/

#include "JoystickLoggerItem.h"
#include <cnoid/Archive>
#include <cnoid/ItemManager>
#include <cnoid/Joystick>
#include <cnoid/MultiValueSeq>
#include <cnoid/MultiValueSeqItem>
#include <cnoid/PutPropertyFunction>
#include <cnoid/SimulatorItem>
#include "gettext.h"

using namespace cnoid;
using namespace std;

namespace cnoid {

class JoystickLoggerItemImpl
{
public:
    JoystickLoggerItemImpl(JoystickLoggerItem* self);
    JoystickLoggerItemImpl(JoystickLoggerItem* self, const JoystickLoggerItemImpl& org);
    JoystickLoggerItem* self;

    vector<Joystick*> joysticks;
    vector<MultiValueSeqItem*> joystickStaSeqItems;
    bool isJoystickStatesRecordingEnabled;
    int frame;

    bool initializeSimulation(SimulatorItem* simulatorItem);
    void onPostDynamicsFunction();
    void doPutProperties(PutPropertyFunction& putProperty);
    bool store(Archive& archive);
    bool restore(const Archive& archive);
};

}


JoystickLoggerItem::JoystickLoggerItem()
{
    impl = new JoystickLoggerItemImpl(this);
}


JoystickLoggerItemImpl::JoystickLoggerItemImpl(JoystickLoggerItem* self)
    : self(self)
{
    joysticks.clear();
    joystickStaSeqItems.clear();
    isJoystickStatesRecordingEnabled = false;
    frame = 0;
}


JoystickLoggerItem::JoystickLoggerItem(const JoystickLoggerItem& org)
    : SubSimulatorItem(org),
      impl(new JoystickLoggerItemImpl(this, *org.impl))
{

}


JoystickLoggerItemImpl::JoystickLoggerItemImpl(JoystickLoggerItem* self, const JoystickLoggerItemImpl& org)
    : self(self)
{
    joysticks = org.joysticks;
    joystickStaSeqItems = org.joystickStaSeqItems;
    isJoystickStatesRecordingEnabled = org.isJoystickStatesRecordingEnabled;
    frame = org.frame;
}


JoystickLoggerItem::~JoystickLoggerItem()
{
    delete impl;
}


void JoystickLoggerItem::initializeClass(ExtensionManager* ext)
{
    ext->itemManager().registerClass<JoystickLoggerItem>(N_("JoystickLoggerItem"));
    ext->itemManager().addCreationPanel<JoystickLoggerItem>();
}


bool JoystickLoggerItem::initializeSimulation(SimulatorItem* simulatorItem)
{
    return impl->initializeSimulation(simulatorItem);
}


bool JoystickLoggerItemImpl::initializeSimulation(SimulatorItem* simulatorItem)
{
    joysticks.clear();
    for(size_t i = 0; i < joystickStaSeqItems.size(); ++i) {
        MultiValueSeqItem* item = joystickStaSeqItems[i];
        item->removeFromParentItem();
    }
    joystickStaSeqItems.clear();
    frame = 0;

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
            shared_ptr<MultiValueSeq> joystickStaSeq = joystickStaSeqItem->seq();
            joystickStaSeq->setSeqContentName("JoystickStaSeq");
            joystickStaSeq->setNumParts(numParts);
            joystickStaSeq->setDimension(0, numParts, 1);
            joystickStaSeq->setFrameRate(1.0 / simulatorItem->worldTimeStep());
        }

        simulatorItem->addPostDynamicsFunction([&](){ onPostDynamicsFunction(); });
    }

    return true;
}


void JoystickLoggerItemImpl::onPostDynamicsFunction()
{
    for(size_t i = 0; i < joysticks.size(); ++i) {
        Joystick* joystick = joysticks[i];
        joystick->readCurrentState();
        shared_ptr<MultiValueSeq> joystickStaSeq = joystickStaSeqItems[i]->seq();
        joystickStaSeq->setNumFrames(frame + 1);
        MultiValueSeq::Frame p = joystickStaSeq->frame(frame);
        for(int i = 0; i < joystick->numAxes(); ++i) {
            p[i] = joystick->getPosition(i);
        }
        for(int i = 0; i < joystick->numButtons(); ++i) {
            p[i + joystick->numAxes()] = joystick->getButtonState(i) ? 1 : 0;
        }
    }

    frame++;
}


Item* JoystickLoggerItem::doDuplicate() const
{
    return new JoystickLoggerItem(*this);
}


void JoystickLoggerItem::doPutProperties(PutPropertyFunction& putProperty)
{
    SubSimulatorItem::doPutProperties(putProperty);
    impl->doPutProperties(putProperty);
}


void JoystickLoggerItemImpl::doPutProperties(PutPropertyFunction& putProperty)
{
    putProperty(_("Record joystick states"), isJoystickStatesRecordingEnabled, changeProperty(isJoystickStatesRecordingEnabled));
}


bool JoystickLoggerItem::store(Archive& archive)
{
    SubSimulatorItem::store(archive);
    return impl->store(archive);
}


bool JoystickLoggerItemImpl::store(Archive& archive)
{
    archive.write("record_joystick_states", isJoystickStatesRecordingEnabled);
    return true;
}


bool JoystickLoggerItem::restore(const Archive& archive)
{
    SubSimulatorItem::restore(archive);
    return impl->restore(archive);
}


bool JoystickLoggerItemImpl::restore(const Archive& archive)
{
    archive.read("record_joystick_states", isJoystickStatesRecordingEnabled);
    return true;
}
