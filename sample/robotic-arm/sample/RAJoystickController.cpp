/**
    RoboticArm Controller
    @author Kenta Suzuki
*/

#include <cnoid/SimpleController>
#include <cnoid/JointPath>
#include <cnoid/EigenUtil>
#include <cnoid/SharedJoystick>
#include <vector>

using namespace std;
using namespace cnoid;

namespace {

const double GEN2_Home[] = {
    103.1, 197.3, 180, 43.6, 265.2, 257.5, 288.1, 0.0, 0.0, 0.0
};

const double GEN3_Home[] = {
    360.0, 15.0, 180.0, 230.0, 360.0, 55.0, 90.0
};

const double GEN3LITE_Home[] = {
    0.0, -20.0, 90.0, 80.0, 60.0, 0.0, 20.0, 0, -20.0, 0.0
};

const double UR3_Home[] = {
    0.0, -150.0, 120.0, -60.0, -90.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0
};

const double UR5_Home[] = {
    0.0, -150.0, 120.0, -60.0, -90.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0
};

const double UR10_Home[] = {
    0.0, -150.0, 120.0, -60.0, -90.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0
};

const double GEN2_Limits[] = {
    1.06, 1.06, 1.06, 1.06, 1.06, 1.06, 1.06, 1.06, 1.06, 1.06
};

const double GEN3_Limits[] = {
    1.39, 1.39, 1.39, 1.39, 1.22, 1.22, 1.22
};

const double GEN3LITE_Limits[] = {
    1.0, 1.0, 1.0, 1.0, 1.57, 1.0, 1.0, 0, 1.0, 1.0
};

const double UR3_Limits[] = {
    1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0
};

const double UR5_Limits[] = {
    1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0
};

const double UR10_Limits[] = {
    1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0
};

}

class RAJoystickController : public SimpleController
{
    SimpleControllerIO* io;

    Body* ioBody;
    Link* ioFinger[3];
    BodyPtr ikBody;
    Link* ikWrist;
    shared_ptr<JointPath> baseToWrist;
    VectorXd qref, qold, qref_old;
    int numFingers;
    int currentMap;
    int prevMap;
    int currentJoint;
    int currentSpeed;
    bool prevActionState;
    double maxV;
    double maxW;
    double timeStep;
    bool isKinematicsMode;

    struct ArmInfo {
        const char* bodyName;
        const char* baseName;
        const char* wristName;
        int numFingers;
        double maxV;
        double maxW;
        const char* finger1Name;
        const char* finger2Name;
        const char* finger3Name;
        const double* homePose;
        const double* speedLimits;
        ArmInfo(const char* bodyName, const char* baseName, const char* wristName, int numFingers, double maxV, double maxW,
            const char* finger1Name, const char* finger2Name, const char* finger3Name, const double* homePose, const double* speedLimits)
            : bodyName(bodyName),
              baseName(baseName),
              wristName(wristName),
              numFingers(numFingers),
              maxV(maxV),
              maxW(maxW),
              finger1Name(finger1Name),
              finger2Name(finger2Name),
              finger3Name(finger3Name),
              homePose(homePose),
              speedLimits(speedLimits)
        { }
    };
    vector<ArmInfo> arms;

    struct ActionInfo {
        int actionId;
        int buttonId;
        bool prevButtonState;
        bool stateChanged;
        ActionInfo(int actionId, int buttonId)
            : actionId(actionId),
              buttonId(buttonId),
              prevButtonState(false),
              stateChanged(false)
        { }
    };
    vector<ActionInfo> actions;
    vector<ActionInfo> actions2;

    SharedJoystickPtr joystick;
    int targetMode;

public:

    virtual bool initialize(SimpleControllerIO* io) override
    {
        this->io = io;
        ostream& os = io->os();

        ioBody = io->body();

        // isKinematicsMode = false;
        isKinematicsMode = true;
        for(auto opt : io->options()) {
            if(opt == "kinematics") {
                isKinematicsMode = true;
            }
        }

        arms = {
            { "Gen2",            "j2s7s300_joint_base", "j2s7s300_joint_end_effector", 3, 0.20, 1.0, "j2s7s300_joint_finger_1",  "j2s7s300_joint_finger_2",   "j2s7s300_joint_finger_3",     GEN2_Home,     GEN2_Limits },
            { "Gen3NV",          "base_joint",          "end_effector",                0, 0.50, 1.0, "",                         "",                          "",                            GEN3_Home,     GEN3_Limits },
            { "Gen3WV",          "base_joint",          "end_effector",                0, 0.50, 1.0, "",                         "",                          "",                            GEN3_Home,     GEN3_Limits },
            { "Gen3lite",        "base_joint",          "end_effector",                2, 0.25, 1.0, "left_finger_bottom_joint", "right_finger_bottom_joint", "",                        GEN3LITE_Home, GEN3LITE_Limits },
            { "UR3-2F85",        "BASE",                "Gripper_Base",                0, 0.25, 1.0, "",                         "",                          "",                             UR3_Home,      UR3_Limits },
            { "UR5-2F85",        "BASE",                "Gripper_Base",                0, 0.50, 1.0, "",                         "",                          "",                             UR5_Home,      UR5_Limits },
            { "Husky-UR5-2F85",  "UR5_BASE",            "UR5_Gripper_Base",            0, 0.50, 1.0, "",                         "",                          "",                             UR5_Home,      UR5_Limits },
            { "UR10-2F85",       "BASE",                "Gripper_Base",                0, 0.50, 1.0, "",                         "",                          "",                            UR10_Home,     UR10_Limits }
        };

        for(auto& info : arms) {
            if(ioBody->name() == info.bodyName) {
                os << info.bodyName << " is found." << endl;
                numFingers = info.numFingers;
                maxV = info.maxV;
                maxW = info.maxW;
                if(info.numFingers == 2) {
                    ioFinger[0] = ioBody->link(info.finger1Name);
                    ioFinger[1] = ioBody->link(info.finger2Name);
                } else if(info.numFingers == 3) {
                    ioFinger[0] = ioBody->link(info.finger1Name);
                    ioFinger[1] = ioBody->link(info.finger2Name);
                    ioFinger[2] = ioBody->link(info.finger3Name);
                }

                ikBody = ioBody->clone();
                ikWrist = ikBody->link(info.wristName);
                Link* base = ikBody->rootLink();
                baseToWrist = JointPath::getCustomPath(base, ikWrist);
                base->p().setZero();
                base->R().setIdentity();
            }
        }

        const int nj = ioBody->numJoints();
        qold.resize(nj);
        for(int i = 0; i < nj; ++i) {
            Link* joint = ioBody->joint(i);
            joint->setActuationMode(isKinematicsMode ? JointDisplacement : JointEffort);
            io->enableIO(joint);
            double q = joint->q();
            ikBody->joint(i)->q() = q;
            qold[i] = q;
        }

        baseToWrist->calcForwardKinematics();
        qref = qold;
        qref_old = qold;

        currentMap = prevMap = 0;
        currentJoint = 0;
        currentSpeed = 50;
        prevActionState = false;
        timeStep = io->timeStep();

        actions = {
            { 0, Joystick::SELECT_BUTTON },
            { 1, Joystick::START_BUTTON  },
            { 2, Joystick::A_BUTTON      },
            { 3, Joystick::B_BUTTON      }
        };
        actions2 = {
            { 0, 0 },
            { 1, 1 }
        };

        joystick = io->getOrCreateSharedObject<SharedJoystick>("joystick");
        targetMode = joystick->addMode();

        if(timeStep < 0.005) {
            os << "timestep < 0.005" << endl;
            return false;
        }

        return true;
    }

    virtual bool control() override
    {
        joystick->updateState(targetMode);

        static const char* texts[] = {
            "twist-linear control has set.", "twist-angular control has set.",
            "joint control has set."
        };

        bool is_action_running = false;
        for(auto& info : actions) {
            bool stateChanged = false;
            bool buttonState = joystick->getButtonState(targetMode, info.buttonId);
            if(buttonState && !info.prevButtonState) {
                stateChanged = true;
            }
            info.prevButtonState = buttonState;
            if(stateChanged) {
                if(info.actionId == 0) {
                    currentMap = currentMap == 0 ? 2 : currentMap - 1;
                    io->os() << texts[currentMap] << endl;
                } else if(info.actionId == 1) {
                    currentMap = currentMap == 2 ? 0 : currentMap + 1;
                    io->os() << texts[currentMap] << endl;
                }
            }

            if(buttonState) {
                if(info.actionId == 2) {

                } else if(info.actionId == 3) {
                    is_action_running = true;
                }
            }
        }

        if(1) {
            double pos[2];
            for(int i = 0; i < 2; ++i) {
                pos[i] = joystick->getPosition(targetMode,
                    i == 0 ? Joystick::DIRECTIONAL_PAD_H_AXIS : Joystick::DIRECTIONAL_PAD_V_AXIS);
                if(fabs(pos[i]) < 0.2) {
                    pos[i] = 0.0;
                }
            }

            bool getPadState[2];
            getPadState[0] = fabs(pos[0]) > 0.0 ? true : false;
            getPadState[1] = fabs(pos[1]) > 0.0 ? true : false;

            for(auto& info : actions2) {
                bool stateChanged = false;
                bool buttonState = getPadState[info.buttonId];
                if(buttonState && !info.prevButtonState) {
                    stateChanged = true;
                }
                info.prevButtonState = buttonState;
                if(stateChanged) {
                    if(info.actionId == 0) {
                        // joint selection
                        if(currentMap == 2) {
                            if(pos[0] == -1) {
                                currentJoint = currentJoint == 0 ? 5 : currentJoint - 1;
                                io->os() << "joint " << currentJoint << " is selected." << endl;
                            } else if(pos[0] == 1) {
                                currentJoint = currentJoint == 5 ? 0 : currentJoint + 1;
                                io->os() << "joint " << currentJoint << " is selected." << endl;
                            }
                        }
                    } else if(info.actionId == 1){
                        // speed selection
                        if(pos[1] == -1) {
                            currentSpeed = currentSpeed == 100 ? 100 : currentSpeed + 10;
                        } else if(pos[1] == 1) {
                            currentSpeed = currentSpeed == 40 ? 40 : currentSpeed - 10;
                        }
                        io->os() << "current speed is " << currentSpeed << "%." << endl;
                    }
                }
            }
        }

        if(is_action_running) {
            for(int i = 0; i < ioBody->numJoints(); ++i) {
                for(auto& info : arms) {
                    if(ioBody->name() == info.bodyName) {
                        Link* joint = ioBody->joint(i);
                        double qe = radian(info.homePose[i]);
                        double q = joint->q();
                        double deltaq = (qe - q) * timeStep;
                        qref[joint->jointId()] += deltaq;
                    }
                }
            }
        }

        bool stateChanged = false;
        if(!is_action_running && prevActionState) {
            stateChanged = true;
        }
        prevActionState = is_action_running;

        if(currentMap != prevMap) {
            stateChanged = true;
        }
        prevMap = currentMap;

        if(stateChanged) {
            for(int i = 0; i < ioBody->numJoints(); ++i) {
                Link* joint = ioBody->joint(i);
                double q = joint->q();
                ikBody->joint(i)->q() = q;
                qold[i] = q;
            }

            baseToWrist->calcForwardKinematics();
        }

        if(!is_action_running) {
            const double speedRatio = (double)currentSpeed / 100.0;

            if(currentMap == 2) {
                double pos = joystick->getPosition(targetMode, Joystick::R_STICK_H_AXIS);
                if(fabs(pos) < 0.15) {
                    pos = 0.0;
                }

                Link* joint = ioBody->joint(currentJoint);
                if((joint->q() < joint->q_lower() && pos < 0.0)
                    || (joint->q() > joint->q_upper() && pos > 0.0)) {
                    pos = 0.0;
                }

                for(auto& info : arms) {
                    if(ioBody->name() == info.bodyName) {
                        // selected joint rotation
                        double w = info.speedLimits[joint->jointId()];
                        double wt = w * timeStep * speedRatio;
                        qref[joint->jointId()] += pos * wt;
                    }
                }
            } else {
                static const int axisID[] = {
                    Joystick::L_STICK_H_AXIS, Joystick::L_STICK_V_AXIS,
                    Joystick::R_STICK_H_AXIS, Joystick::R_STICK_V_AXIS
                };

                double pos[4];
                for(int i = 0; i < 4; ++i) {
                    pos[i] = joystick->getPosition(targetMode, axisID[i]);
                    if(fabs(pos[i]) < 0.2) {
                        pos[i] = 0.0;
                    }
                }

                VectorXd p(6);
                if(currentMap == 0) {
                    Vector3 vt = Vector3(-pos[1], -pos[0], -pos[3]) * maxV * timeStep * speedRatio;
                    p.head<3>() = ikWrist->p() + vt;
                    p.tail<3>() = rpyFromRot(ikWrist->R());
                } else if(currentMap == 1) {
                    Vector3 wt = Vector3(pos[1], -pos[0], -pos[2]) * maxW * timeStep * speedRatio;
                    p.head<3>() = ikWrist->p();
                    p.tail<3>() = rpyFromRot(ikWrist->R() * rotFromRpy(wt));
                }

                Isometry3 T;
                T.linear() = rotFromRpy(Vector3(p.tail<3>()));
                T.translation() = p.head<3>();
                if(baseToWrist->calcInverseKinematics(T)) {
                    for(int i = 0; i < baseToWrist->numJoints(); ++i) {
                        Link* joint = baseToWrist->joint(i);
                        qref[joint->jointId()] = joint->q();
                    }
                }
            }
        }

        if(numFingers > 0) {
            double pos[2];
            for(int i = 0; i < 2; ++i) {
                pos[i] = joystick->getPosition(targetMode,
                    i == 0 ? Joystick::L_TRIGGER_AXIS : Joystick::R_TRIGGER_AXIS);
                if(fabs(pos[i]) < 0.2) {
                    pos[i] = 0.0;
                }
            }

            for(int i = 0; i < numFingers; ++i) {
                double dq_hand = 0.0;
                if(fabs(pos[0]) > 0.0) {
                    dq_hand = 1.0;
                } else if(fabs(pos[1]) > 0.0) {
                    dq_hand = -1.0;
                }
                if(ioFinger[i]->name() == "right_finger_bottom_joint") {
                    dq_hand *= -1.0;
                }
                if((ioFinger[i]->q() < ioFinger[i]->q_lower() && dq_hand < 0.0)
                    || (ioFinger[i]->q() > ioFinger[i]->q_upper() && dq_hand > 0.0)) {
                        dq_hand = 0.0;
                }
                qref[ioFinger[i]->jointId()] += radian(dq_hand);
            }
        }

        for(int i = 0; i < ioBody->numJoints(); ++i) {
            if(isKinematicsMode) {
                ioBody->joint(i)->q_target() = qref[i];
            } else {
                double q = ioBody->joint(i)->q();
                double dq = (q - qold[i]) / timeStep;
                double dq_ref = (qref[i] - qref_old[i]) / timeStep;
                // ioBody->joint(i)->u() = (qref[i] - q) * pgain[i] + (dq_ref - dq) * dgain[i];
                qold[i] = q;
            }
        }
        qref_old = qref;

        return true;
    }
};

CNOID_IMPLEMENT_SIMPLE_CONTROLLER_FACTORY(RAJoystickController)