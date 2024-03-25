/**
   @author Kenta Suzuki
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

class JoystickLoggerItem::Impl
{
public:
    JoystickLoggerItem* self;

    Impl(JoystickLoggerItem* self);
    Impl(JoystickLoggerItem* self, const Impl& org);

    string device;
    Joystick* joystick;
    MultiValueSeqItemPtr joystickStateSeqItem;
    SimulatorItem* simulatorItem;

    bool initializeSimulation(SimulatorItem* simulatorItem);
    void onPostDynamics();
};

}


JoystickLoggerItem::JoystickLoggerItem()
{
    impl = new Impl(this);
}


JoystickLoggerItem::Impl::Impl(JoystickLoggerItem* self)
    : self(self)
{
    device = "/dev/input/js0";
    joystickStateSeqItem = nullptr;
    simulatorItem = nullptr;
}


JoystickLoggerItem::JoystickLoggerItem(const JoystickLoggerItem& org)
    : SubSimulatorItem(org),
      impl(new Impl(this, *org.impl))
{

}


JoystickLoggerItem::Impl::Impl(JoystickLoggerItem* self, const Impl& org)
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
        .registerClass<JoystickLoggerItem, SubSimulatorItem>(N_("JoystickLoggerItem"))
        .addCreationPanel<JoystickLoggerItem>();
}


bool JoystickLoggerItem::initializeSimulation(SimulatorItem* simulatorItem)
{
    return impl->initializeSimulation(simulatorItem);
}


bool JoystickLoggerItem::Impl::initializeSimulation(SimulatorItem* simulatorItem)
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
        simulatorItem->addPostDynamicsFunction([&](){ onPostDynamics(); });
    }
    return true;
}


void JoystickLoggerItem::Impl::onPostDynamics()
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


Item* JoystickLoggerItem::doCloneItem(CloneMap* cloneMap) const
{
    return new JoystickLoggerItem(*this);
}


void JoystickLoggerItem::doPutProperties(PutPropertyFunction& putProperty)
{
    SubSimulatorItem::doPutProperties(putProperty);
    putProperty(_("Device"), impl->device, changeProperty(impl->device));
}


bool JoystickLoggerItem::store(Archive& archive)
{
    SubSimulatorItem::store(archive);
    archive.write("device", impl->device);
    return true;
}


bool JoystickLoggerItem::restore(const Archive& archive)
{
    SubSimulatorItem::restore(archive);
    archive.read("device", impl->device);
    return true;
}
