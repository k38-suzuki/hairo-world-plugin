/**
   @author Kenta Suzuki
*/

#ifndef CNOID_BEEP_PLUGIN_BEEPER_H
#define CNOID_BEEP_PLUGIN_BEEPER_H

#include <cnoid/Signal>
#include "exportdecl.h"

namespace cnoid {

class CNOID_EXPORT Beeper
{
public:

    Beeper();
    virtual ~Beeper();

    bool isActive() const;
    void start(const int& frequency = 440, const int& length = 200);

    SignalProxy<void()> sigBeepStarted();
    SignalProxy<void()> sigBeepStopped();

private:
    class Impl;
    Impl* impl;
};

}

#endif // CNOID_BEEP_PLUGIN_BEEPER_H