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

using namespace std;
using namespace cnoid;

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
    : SubSimulatorItem()
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
    shared_ptr<MultiValueSeq> log = joystickStateSeqItem->seq();
    log->setNumFrames(0);
    log->setNumParts(numParts);
    log->setFrameRate(1.0 / simulatorItem->worldTimeStep());
    log->setOffsetTime(0.0);

    if(simulatorItem) {
        simulatorItem->addPostDynamicsFunction([&](){ onPostDynamics(); });
    }
    return true;
}


void JoystickLoggerItem::Impl::onPostDynamics()
{
    int currentFrame = simulatorItem->currentFrame();
    joystick->readCurrentState();
    shared_ptr<MultiValueSeq> log = joystickStateSeqItem->seq();
    auto frame = log->appendFrame();
    for(int i = 0; i < joystick->numAxes(); ++i) {
        frame[i] = joystick->getPosition(i);
    }
    for(int i = 0; i < joystick->numButtons(); ++i) {
        frame[i + joystick->numAxes()] = joystick->getButtonState(i) ? 1 : 0;
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
    if(!SubSimulatorItem::store(archive)) {
        return false;
    }
    archive.write("device", impl->device);
    return true;
}


bool JoystickLoggerItem::restore(const Archive& archive)
{
    if(!SubSimulatorItem::restore(archive)) {
        return false;
    }
    archive.read("device", impl->device);
    return true;
}
