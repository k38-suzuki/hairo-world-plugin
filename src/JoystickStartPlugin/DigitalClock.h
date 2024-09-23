/**
    @author Kenta Suzuki
*/

#ifndef CNOID_JOYSTICKSTART_PLUGIN_DIGITAL_CLOCK_H
#define CNOID_JOYSTICKSTART_PLUGIN_DIGITAL_CLOCK_H

#include <QLCDNumber>

namespace cnoid {

class ExtensionManager;

class DigitalClock : public QLCDNumber
{
public:
    static void initializeClass(ExtensionManager* ext);

    DigitalClock(QWidget* parent = nullptr);
    virtual ~DigitalClock();

private:
    class Impl;
    Impl* impl;
};

}

#endif // CNOID_JOYSTICKSTART_PLUGIN_DIGITAL_CLOCK_H