/**
   @author Kenta Suzuki
*/

#ifndef CNOID_OMPL_PLUGIN_SIMPLE_SETUP_H
#define CNOID_OMPL_PLUGIN_SIMPLE_SETUP_H

#include <cnoid/BoundingBox>
#include <cnoid/EigenTypes>
#include <ompl/geometric/SimpleSetup.h>
#include "exportdecl.h"

namespace cnoid {

class CNOID_EXPORT SimpleSetup
{
public:
    SimpleSetup();
    SimpleSetup(const SimpleSetup& org);
    virtual ~SimpleSetup();

    enum PlannerType {
        RRT,
        RRTCONNECT,
        RRTSTAR,
        PRRT,
        NUM_PLANNERS
    };

    void setPlanner(const int& planner);
    int planner() const;
    void setBoundingBox(const BoundingBox& bb);
    BoundingBox boundingBox() const;
    void setStartPosition(const Vector3& startPosition);
    Vector3 startPosition() const;
    void setGoalPosition(const Vector3& goalPosition);
    Vector3 goalPosition() const;
    void setCalculationTime(const double& calculationTime);
    double calculationTime() const;

    void planWithSimpleSetup();
    bool isSolved() const;

protected:
    virtual void prePlannerFunction() = 0;
    virtual bool midPlannerFunction(const ompl::base::State* state) = 0;
    virtual void postPlannerFunction(ompl::geometric::PathGeometric& pathes) = 0;

private:
    class Impl;
    Impl* impl;
};

}

#endif
