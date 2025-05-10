/**
    @author Kenta Suzuki
*/

#ifndef CNOID_CFD_PLUGIN_SIMPLE_PILOT_H
#define CNOID_CFD_PLUGIN_SIMPLE_PILOT_H

#include <cnoid/EigenUtil>
#include <cnoid/RateGyroSensor>
#include <cnoid/SimpleController>
#include "exportdecl.h"

namespace cnoid {

class CNOID_EXPORT SimplePilot
{
public:
    SimplePilot();
    ~SimplePilot();

    enum StickMode { MODE1, MODE2 };

    enum FlightControl { THROTTLE, AILERON, ELEVATOR, RUDDER };

    bool initialize(SimpleControllerIO* io);
    bool findRateGyroSensor(const std::string& name);
    bool readCurrentState();

    void on(bool on) { on_ = on; }

    bool on() { return on_; }

    void setStickMode(int mode) { mode_ = mode; }

    bool switchMode() const;
    int axisId(int controlId);
    double gravityCompensation(int nor); // Quadcopter: nor = 4

    Vector4 zrpy();

    Vector4 dzrpy() { return dz; }

    Vector2 dxy_local() { return dx_local; }

    Vector2 ddxy_local() { return ddx_local; }

private:
    Vector2 xy();
    Vector2 local(Vector2 xy);

    SimpleControllerIO* io;
    BodyPtr ioBody;
    RateGyroSensor* gyroSensor;
    Vector4 dz;
    Vector4 zold;
    Vector2 xold;
    Vector2 dxold;
    Vector2 dx_local;
    Vector2 ddx_local;
    bool on_;
    int mode_;
    double timeStep;
};

typedef SimplePilot::StickMode StickMode;
typedef SimplePilot::FlightControl FlightControl;

} // namespace cnoid

#endif