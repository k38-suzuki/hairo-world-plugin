/**
   @author Kenta Suzuki
*/

#ifndef CNOID_JOYSTICKSTART_PLUGIN_JOYSTICK_LOGGER_ITEM_H
#define CNOID_JOYSTICKSTART_PLUGIN_JOYSTICK_LOGGER_ITEM_H

#include <cnoid/SubSimulatorItem>

namespace cnoid {

class JoystickLoggerItem : public SubSimulatorItem
{
public:
    JoystickLoggerItem();
    JoystickLoggerItem(const JoystickLoggerItem& org);
    virtual ~JoystickLoggerItem();

    static void initializeClass(ExtensionManager* ext);
    virtual bool initializeSimulation(SimulatorItem* simulatorItem) override;

protected:
    virtual Item* doCloneItem(CloneMap* cloneMap) const override;
    virtual void doPutProperties(PutPropertyFunction& putProperty) override;
    virtual bool store(Archive& archive) override;
    virtual bool restore(const Archive& archive) override;

private:
    class Impl;
    Impl* impl;
};

typedef ref_ptr<JoystickLoggerItem> JoystickLoggerItemPtr;

}

#endif // CNOID_JOYSTICKSTART_PLUGIN_JOYSTICK_LOGGER_ITEM_H
