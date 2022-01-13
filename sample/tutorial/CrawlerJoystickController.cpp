/**
   Crawler Controller
   @author Kenta Suzuki
*/

#include <cnoid/EigenUtil>
#include <cnoid/Joystick>
#include <cnoid/SimpleController>

using namespace cnoid;
using namespace std;

class CrawlerJoystickController : public SimpleController
{
    SimpleControllerIO* io;

    enum TrackID {
        TRACK_L, TRACK_R, TRACK_LF,
        TRACK_RF, TRACK_LR, TRACK_RR,
        NUM_TRACKS
    };

    enum JointID {
        SPACER_LF, SPACER_RF, SPACER_LR,
        SPACER_RR, NUM_JOINTS
    };

    Link* tracks[NUM_TRACKS];
    Link* joints[NUM_JOINTS];
    Joystick* joystick;
    string device;

public:

    virtual bool initialize(SimpleControllerIO* io) override
    {
        this->io = io;
        ostream& os = io->os();
        Body* body = io->body();
        device = "/dev/input/js0";

        for(auto opt : io->options()) {
            if(opt == "2p") {
                device = "/dev/input/js1";
            }
        }

        joystick = new Joystick(device.c_str());

        static const char* tracknames[] = {
            "TRACK_L", "TRACK_R", "TRACK_LF",
            "TRACK_RF", "TRACK_LR", "TRACK_RR",
        };

        for(int i = 0; i < NUM_TRACKS; ++i) {
            Link* track = body->link(tracknames[i]);
            tracks[i] = track;
            if(!track) {
                os << "Track" << tracknames[i] << " is not found." << endl;
            } else {
                io->enableOutput(track, JointVelocity);
            }
        }

        static const char* jointnames[] = {
            "SPACER_LF", "SPACER_RF", "SPACER_LR", "SPACER_RR"
        };

        for(int i = 0; i < NUM_JOINTS; ++i) {
            Link* joint = body->link(jointnames[i]);
            joints[i] = joint;
            if(!joint) {
                os << "Joint" << jointnames[i] << " is not found." << endl;
            } else {
                joint->setActuationMode(JointVelocity);
                io->enableIO(joint);
            }
        }

        return true;
    }

    virtual bool control() override
    {
        joystick->readCurrentState();

        double pos[2] = { 0.0, 0.0 };
        for(int i = 0; i < 2; ++i) {
            pos[i] = joystick->getPosition(
                        i == 0 ? Joystick::L_STICK_H_AXIS : Joystick::L_STICK_V_AXIS);
            if(fabs(pos[i]) < 0.2) {
                pos[i] = 0.0;
            }
        }

        for(int i = 0; i < NUM_TRACKS / 2; ++i) {
            double k = 0.5;
            Link* trackL = tracks[2 * i];
            Link* trackR = tracks[2 * i + 1];
            if(trackL) {
                trackL->dq_target() = k * (-2.0 * pos[1] + pos[0]);
            }
            if(trackR) {
                trackR->dq_target() = k * (-2.0 * pos[1] - pos[0]);
            }
        }

        for(int i = 0; i < 2; ++i) {
            double pos = 0.0;;
            bool on = joystick->getButtonState(
                        i == 0 ? Joystick::L_BUTTON : Joystick::R_BUTTON);
            if(!on) {
                pos = joystick->getPosition(
                            i == 0 ? Joystick::L_TRIGGER_AXIS : Joystick::R_TRIGGER_AXIS);
                pos *= i == 0 ? -1.0 : 1.0;
            } else {
                pos = i == 0 ? 1.0 : -1.0;
            }
            if(fabs(pos) < 0.2) {
                pos = 0.0;
            }

            static const double ranges[] = { -90.0, 90.0 };
            for(int j = 0; j < 2; ++j) {
                Link* joint = joints[2 * i + j];
                if(joint) {
                    double q = joint->q();
                    if((q > radian(ranges[1]) && pos > 0.0)
                            || (q < radian(ranges[0]) && pos < 0.0)) {
                        pos = 0.0;
                    }
                    joint->dq_target() = pos;
                }
            }
        }

        return true;
    }
};

CNOID_IMPLEMENT_SIMPLE_CONTROLLER_FACTORY(CrawlerJoystickController)
