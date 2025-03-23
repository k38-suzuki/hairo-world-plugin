/**
   Crawler Controller
   @author Kenta Suzuki
*/

#include <cnoid/EigenUtil>
#include <cnoid/Joystick>
#include <cnoid/SimpleController>

using namespace std;
using namespace cnoid;

class CrawlerJoystickController : public SimpleController
{
    SimpleControllerIO* io;
    bool usePseudoContinousTrackMode;

    enum {
        TRACK_L, TRACK_R, TRACK_LF,
        TRACK_RF, TRACK_LR, TRACK_RR,
        NumTracks
    };

    enum {
        SPACER_LF, SPACER_RF, SPACER_LR,
        SPACER_RR, NumJoints
    };

    Link* track[NumTracks];
    Link* spacer[NumJoints];

    Joystick* joystick;

public:

    virtual bool initialize(SimpleControllerIO* io) override
    {
        this->io = io;
        ostream& os = io->os();
        Body* body = io->body();
        string device = "/dev/input/js0";

        usePseudoContinousTrackMode = true;
        for(auto opt : io->options()) {
            if(opt == "wheels") {
                usePseudoContinousTrackMode = false;
            }
            if(opt == "2p") {
                device = "/dev/input/js1";
            }
        }

        joystick = new Joystick(device.c_str());

        static const char* track_name[] = {
            "TRACK_L", "TRACK_R", "TRACK_LF",
            "TRACK_RF", "TRACK_LR", "TRACK_RR",
        };

        static const char* agxtrack_name[] = {
            "SPROCKET_L", "SPROCKET_R", "SPROCKET_LF",
            "SPROCKET_RF", "SPROCKET_LR", "SPROCKET_RR",
        };

        for(int i = 0; i < NumTracks; ++i) {
            track[i] = body->link(track_name[i]);
            if(!usePseudoContinousTrackMode) {
                track[i] = body->link(agxtrack_name[i]);
            }
            if(!track[i]) {
                os << "Track" << i << " is not found." << endl;
            } else {
                io->enableOutput(track[i], JointVelocity);
            }
        }

        static const char* joint_name[] = {
            "SPACER_LF", "SPACER_RF", "SPACER_LR", "SPACER_RR"
        };

        for(int i = 0; i < NumJoints; ++i) {
            Link* joint = body->link(joint_name[i]);
            spacer[i] = joint;
            if(!joint) {
                os << "Joint" << i << " is not found." << endl;
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

        double pos[2] = { 0.0 };
        for(int i = 0; i < 2; ++i) {
            pos[i] = joystick->getPosition(
                        i == 0 ? Joystick::L_STICK_H_AXIS : Joystick::L_STICK_V_AXIS);
            if(fabs(pos[i]) < 0.2) {
                pos[i] = 0.0;
            }
        }

        for(int i = 0; i < NumTracks / 2; ++i) {
            Link* trackL = track[2 * i];
            Link* trackR = track[2 * i + 1];
            if(usePseudoContinousTrackMode) {
                double k = 1.0;
                if(trackL) {
                    trackL->dq_target() = k * (-2.0 * pos[1] + pos[0]);
                }
                if(trackR) {
                    trackR->dq_target() = k * (-2.0 * pos[1] - pos[0]);
                }
            } else {
                double k = 4.0;
                if(trackL) {
                    trackL->dq_target() = k * (-pos[1] + pos[0]);
                }
                if(trackR) {
                    trackR->dq_target() = k * (-pos[1] - pos[0]);
                }
            }
        }

        for(int i = 0; i < 2; ++i) {
            double pos = 0.0;
            bool buttonState = joystick->getButtonState(
                        i == 0 ? Joystick::L_BUTTON : Joystick::R_BUTTON);
            if(!buttonState) {
                pos = joystick->getPosition(
                            i == 0 ? Joystick::L_TRIGGER_AXIS : Joystick::R_TRIGGER_AXIS);
                pos *= i == 0 ? -1.0 : 1.0;
            } else {
                pos = i == 0 ? 1.0 : -1.0;
            }
            if(fabs(pos) < 0.2) {
                pos = 0.0;
            }

            static const double range[] = { -90.0, 90.0 };
            for(int j = 0; j < 2; ++j) {
                Link* joint = spacer[2 * i + j];
                if(joint) {
                    double q = joint->q();
                    if((q > radian(range[1]) && pos > 0.0)
                            || (q < radian(range[0]) && pos < 0.0)) {
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