/**
   @author Kenta Suzuki
*/

#include "Beeper.h"
#include <cnoid/Process>
#include <cnoid/Timer>

using namespace std;
using namespace cnoid;

namespace cnoid {

class Beeper::Impl
{
public:

    Impl();
    ~Impl();

    Timer* timer;
    Process process;
    Signal<void()> sigBeepStarted;
    Signal<void()> sigBeepStopped;

    bool is_active;

    void start(const int& frequency, const int& length);
    bool terminate();
    void onTimeout();
};

}


Beeper::Beeper()
{
    impl = new Impl;
}


Beeper::Impl::Impl()
    : is_active(false)
{
    timer = new Timer;
    timer->sigTimeout().connect([&](){ onTimeout(); });
}


Beeper::~Beeper()
{
    delete impl;
}


Beeper::Impl::~Impl()
{
    terminate();
}


bool Beeper::isActive() const
{
    return impl->is_active;
}


void Beeper::start(const int& frequency, const int& length)
{
    impl->start(frequency, length);
}


void Beeper::Impl::start(const int& frequency, const int& length)
{
    QString program = "beep";
    QStringList arguments;
    if(frequency > 0) {
        arguments << QString("-f %1").arg(frequency);
    }
    if(length > 0) {
        arguments << QString("-l %1").arg(length);
    }

    terminate();
    process.start(program, arguments);

    if(process.waitForStarted()) {
        timer->start(length);
        sigBeepStarted();
        is_active = true;
    }
}


bool Beeper::Impl::terminate()
{
    if(timer->isActive()) {
        timer->stop();
    }

    if(process.state() != Process::NotRunning) {
        process.kill();
        return process.waitForFinished(100);
    }
    return false;
}


void Beeper::Impl::onTimeout()
{
    timer->stop();
    sigBeepStopped();
    is_active = false;
}


SignalProxy<void()> Beeper::sigBeepStarted()
{
    return impl->sigBeepStarted;
}



SignalProxy<void()> Beeper::sigBeepStopped()
{
    return impl->sigBeepStopped;
}