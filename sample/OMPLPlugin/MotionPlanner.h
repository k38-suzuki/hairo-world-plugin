/**
   \file
   \author Kenta Suzuki
*/

#ifndef CNOID_OMPLPLUGIN_MOTIONPLANNER_H
#define CNOID_OMPLPLUGIN_MOTIONPLANNER_H

#include <cnoid/ExtensionManager>

namespace cnoid {

class MotionPlannerImpl;

class MotionPlanner
{
public:
    MotionPlanner(ExtensionManager* ext);
    virtual ~MotionPlanner();

    static void initialize(ExtensionManager* ext);

private:
    MotionPlannerImpl* impl;
    friend class MotionPlannerImpl;
};

}

#endif // CNOID_OMPLPLUGIN_MOTIONPLANNER_H