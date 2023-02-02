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
#include <QDateTime>
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

    string device;
    Joystick* joystick;
    MultiValueSeqItemPtr joystickStateSeqItem;
    SimulatorItem* simulatorItem;

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
    device = "/dev/input/js0";
    joystickStateSeqItem = nullptr;
    simulatorItem = nullptr;
}


JoystickLoggerItem::JoystickLoggerItem(const JoystickLoggerItem& org)
    : SubSimulatorItem(org),
      impl(new JoystickLoggerItemImpl(this, *org.impl))
{

}


JoystickLoggerItemImpl::JoystickLoggerItemImpl(JoystickLoggerItem* self, const JoystickLoggerItemImpl& org)
    : self(self)
{
    device = org.device;
}


JoystickLoggerItem::~JoystickLoggerItem()
{
    delete impl;
}


void JoystickLoggerItem::initializeClass(ExtensionManager* ext)
{
    ext->itemManager()
            .registerClass<JoystickLoggerItem>(N_("JoystickLoggerItem"))
            .addCreationPanel<JoystickLoggerItem>();
}


bool JoystickLoggerItem::initializeSimulation(SimulatorItem* simulatorItem)
{
    return impl->initializeSimulation(simulatorItem);
}


bool JoystickLoggerItemImpl::initializeSimulation(SimulatorItem* simulatorItem)
{
    this->simulatorItem = simulatorItem;
    joystick = new Joystick(device.c_str());
    joystickStateSeqItem = new MultiValueSeqItem;
    QDateTime loggingStartTime = QDateTime::currentDateTime();
    string suffix = loggingStartTime.toString("yyyy-MM-dd-hh-mm-ss").toStdString();
    string name = suffix + " - " + device;
    joystickStateSeqItem->setName(name);
    self->addSubItem(joystickStateSeqItem);

    int numParts = joystick->numAxes() + joystick->numButtons();
    shared_ptr<MultiValueSeq> joystickStateSeq = joystickStateSeqItem->seq();
    joystickStateSeq->setSeqContentName("JoystickStaSeq");
    joystickStateSeq->setFrameRate(1.0 / simulatorItem->worldTimeStep());
    joystickStateSeq->setDimension(0, numParts, false);
    joystickStateSeq->setOffsetTime(0.0);

    if(simulatorItem) {
        simulatorItem->addPostDynamicsFunction([&](){ onPostDynamicsFunction(); });
    }
    return true;
}


void JoystickLoggerItemImpl::onPostDynamicsFunction()
{
    int currentFrame = simulatorItem->currentFrame();
    joystick->readCurrentState();
    shared_ptr<MultiValueSeq> joystickStateSeq = joystickStateSeqItem->seq();
    joystickStateSeq->setNumFrames(currentFrame);
    MultiValueSeq::Frame p = joystickStateSeq->frame(currentFrame - 1);
    for(int i = 0; i < joystick->numAxes(); ++i) {
        p[i] = joystick->getPosition(i);
    }
    for(int i = 0; i < joystick->numButtons(); ++i) {
        p[i + joystick->numAxes()] = joystick->getButtonState(i) ? 1 : 0;
    }
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
    putProperty(_("Device"), device, changeProperty(device));
}


bool JoystickLoggerItem::store(Archive& archive)
{
    SubSimulatorItem::store(archive);
    return impl->store(archive);
}


bool JoystickLoggerItemImpl::store(Archive& archive)
{
    archive.write("device", device);
    return true;
}


bool JoystickLoggerItem::restore(const Archive& archive)
{
    SubSimulatorItem::restore(archive);
    return impl->restore(archive);
}


bool JoystickLoggerItemImpl::restore(const Archive& archive)
{
    archive.read("device", device);
    return true;
}
