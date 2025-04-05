#include <cnoid/SimpleController>
#include <cnoid/Joystick>

using namespace cnoid;

class TrackController2 : public SimpleController
{
    Link* trackL[2];
    Link* trackR[2];
    Joystick joystick;

public:
    virtual bool initialize(SimpleControllerIO* io) override
    {
        trackL[0] = io->body()->link("TRACK_L");
        trackR[0] = io->body()->link("TRACK_R");
        trackL[1] = io->body()->link("TRACK_FL");
        trackR[1] = io->body()->link("TRACK_FR");

        for(int i = 0; i < 2; ++i) {
            trackL[i]->setActuationMode(Link::JointVelocity);
            trackR[i]->setActuationMode(Link::JointVelocity);

            io->enableOutput(trackL[i]);
            io->enableOutput(trackR[i]);
        }

        return true;
    }

    virtual bool control() override
    {
        static const int axisID[] = { 0, 1 };

        joystick.readCurrentState();

        double pos[2];
        for(int i = 0; i < 2; ++i) {
            pos[i] = joystick.getPosition(axisID[i]);
            if(fabs(pos[i]) < 0.2) {
                pos[i] = 0.0;
            }
        }

        for(int i = 0; i < 2; ++i) {
            trackL[i]->dq_target() = -2.0 * pos[1] + pos[0];
            trackR[i]->dq_target() = -2.0 * pos[1] - pos[0];
        }

        return true;
    }
};

CNOID_IMPLEMENT_SIMPLE_CONTROLLER_FACTORY(TrackController2)