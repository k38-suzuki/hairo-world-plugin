/**
    @author Kenta Suzuki
*/

#include "SimplePilot.h"
#include <cnoid/Joystick>

using namespace cnoid;

SimplePilot::SimplePilot()
    : on_(true),
      mode_(StickMode::MODE1),
      timeStep(0.0)
{
    ioBody = nullptr;
    gyroSensor = nullptr;
    dz << 0.0, 0.0, 0.0;
    zold << 0.0, 0.0, 0.0;
    xold << 0.0, 0.0;
    dxold << 0.0, 0.0;
    dx_local << 0.0, 0.0;
    ddx_local << 0.0, 0.0;
}

SimplePilot::~SimplePilot()
{
}

bool SimplePilot::initialize(SimpleControllerIO* io)
{
    this->io = io;
    ioBody = io->body();

    if(!ioBody) {
        return false;
    }
    io->enableInput(ioBody->rootLink(), Link::LinkPosition);

    for(auto opt : io->options()) {
        if(opt == "mode1") {
            this->setStickMode(StickMode::MODE1);
        } else if(opt == "mode2") {
            this->setStickMode(StickMode::MODE2);
        }
    }

    zold = this->zrpy();
    xold = this->xy();
    dxold = Vector2::Zero();

    timeStep = io->timeStep();

    return true;
}

bool SimplePilot::findRateGyroSensor(const std::string& name)
{
    if(!ioBody) {
        return false;
    }

    gyroSensor = ioBody->findDevice<RateGyroSensor>(name);
    if(!gyroSensor) {
        return false;
    }
    io->enableInput(gyroSensor);

    return true;
}

bool SimplePilot::readCurrentState()
{
    Vector4 z = this->zrpy();
    dz = (z - zold) / timeStep;
    if(gyroSensor) {
        Vector3 w = ioBody->rootLink()->R() * gyroSensor->w();
        dz[3] = w[2];
    }

    Vector2 x = this->xy();
    Vector2 dx = (x - xold) / timeStep;
    Vector2 ddx = (dx - dxold) / timeStep;
    dx_local = this->local(dx);
    ddx_local = this->local(ddx);

    zold = z;
    xold = x;
    dxold = dx;

    return true;
}

bool SimplePilot::switchMode() const
{
    mode_ == (mode_ == StickMode::MODE1) ? StickMode::MODE2 : StickMode::MODE1;
    return true;
}

int SimplePilot::axisId(int controlId)
{
    static const int mode1ID[] = {
        Joystick::R_STICK_V_AXIS, Joystick::R_STICK_H_AXIS, Joystick::L_STICK_V_AXIS, Joystick::L_STICK_H_AXIS
    };
    static const int mode2ID[] = {
        Joystick::L_STICK_V_AXIS, Joystick::R_STICK_H_AXIS, Joystick::R_STICK_V_AXIS, Joystick::L_STICK_H_AXIS
    };
    return (mode_ == StickMode::MODE1) ? mode1ID[controlId] : mode2ID[controlId];
}

double SimplePilot::gravityCompensation(int nor)
{
    Vector4 z = this->zrpy();
    double cc = cos(z[1]) * cos(z[2]);
    double gc = ioBody->mass() * 9.80665 / (double)nor / cc;
    return gc;
}

Vector4 SimplePilot::zrpy()
{
    auto T = ioBody->rootLink()->position();
    double z = T.translation().z();
    Vector3 rpy = rpyFromRot(T.rotation());
    return Vector4(z, rpy[0], rpy[1], rpy[2]);
}

Vector2 SimplePilot::xy()
{
    auto p = ioBody->rootLink()->translation();
    return Vector2(p.x(), p.y());
}

Vector2 SimplePilot::local(Vector2 xy)
{
    Vector4 z = this->zrpy();
    return Eigen::Rotation2Dd(-z[3]) * xy;
}