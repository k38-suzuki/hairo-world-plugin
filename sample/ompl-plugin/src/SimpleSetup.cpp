/**
   @author Kenta Suzuki
*/

#include "SimpleSetup.h"
#include <cnoid/BoundingBox>
#include <cnoid/MessageView>
#include <ompl/base/SpaceInformation.h>
#include <ompl/base/spaces/SE3StateSpace.h>
#include <ompl/geometric/planners/rrt/RRT.h>
#include <ompl/geometric/planners/rrt/RRTConnect.h>
#include <ompl/geometric/planners/rrt/RRTstar.h>
#include <ompl/geometric/planners/rrt/pRRT.h>
#include <ompl/config.h>
#include "gettext.h"

using namespace cnoid;

namespace ob = ompl::base;
namespace og = ompl::geometric;

namespace cnoid {

class SimpleSetup::Impl
{
public:
    SimpleSetup* self;

    Impl(SimpleSetup* self);
    Impl(SimpleSetup* self, const Impl& org);

    int planner;
    BoundingBox bb;
    Vector3 startPosition;
    Vector3 goalPosition;
    double calculationTime;
    bool isSolved;
    MessageView* mv;

    void planWithSimpleSetup();
    bool isStateValid(const ob::State* state);
};

}


SimpleSetup::SimpleSetup()
{
    impl = new Impl(this);
}


SimpleSetup::Impl::Impl(SimpleSetup* self)
    : self(self),
      mv(MessageView::instance())
{
    planner = SimpleSetup::RRT;
    Vector3 min(-5.0, -5.0, -5.0);
    Vector3 max(5.0, 5.0, 5.0);
    bb.set(min, max);
    startPosition << 0.0, 0.0, 0.0;
    goalPosition << 0.0, 0.0, 0.0;
    calculationTime = 1.0;
    isSolved = false;
}


SimpleSetup::SimpleSetup(const SimpleSetup& org)
    : impl(new Impl(this, *org.impl))
{

}


SimpleSetup::Impl::Impl(SimpleSetup* self, const Impl& org)
    : self(self),
      mv(MessageView::instance())
{
    planner = org.planner;
    bb = org.bb;
    startPosition = org.startPosition;
    goalPosition = org.goalPosition;
    calculationTime = org.calculationTime;
}


SimpleSetup::~SimpleSetup()
{
    delete impl;
}


void SimpleSetup::setPlanner(const int& planner)
{
    impl->planner = planner;
}


int SimpleSetup::planner() const
{
    return impl->planner;
}


void SimpleSetup::setBoundingBox(const BoundingBox& bb)
{
    impl->bb = bb;
}


BoundingBox SimpleSetup::boundingBox() const
{
    return impl->bb;
}


void SimpleSetup::setStartPosition(const Vector3& startPosition)
{
    impl->mv->putln(_("Start position has set."));
    impl->startPosition = startPosition;
}


Vector3 SimpleSetup::startPosition() const
{
    return impl->startPosition;
}


void SimpleSetup::setGoalPosition(const Vector3& goalPosition)
{
    impl->mv->putln(_("Goal position has set."));
    impl->goalPosition = goalPosition;
}


Vector3 SimpleSetup::goalPosition() const
{
    return impl->goalPosition;
}


void SimpleSetup::setCalculationTime(const double& calculationTime)
{
    impl->calculationTime = calculationTime;
}


double SimpleSetup::calculationTime() const
{
    return impl->calculationTime;
}


void SimpleSetup::planWithSimpleSetup()
{
    impl->planWithSimpleSetup();
}


void SimpleSetup::Impl::planWithSimpleSetup()
{
    self->prePlannerFunction();
    isSolved = false;

    auto space(std::make_shared<ob::SE3StateSpace>());

    ob::RealVectorBounds bounds(3);
    bounds.setLow(0, bb.min()[0]);
    bounds.setHigh(0, bb.max()[0]);
    bounds.setLow(1, bb.min()[1]);
    bounds.setHigh(1, bb.max()[1]);
    bounds.setLow(2, bb.min()[2]);
    bounds.setHigh(2, bb.max()[2]);
    space->setBounds(bounds);

    og::SimpleSetup ss(space);

    ss.setStateValidityChecker([&](const ob::State* state) { return isStateValid(state); });

    ob::ScopedState<ob::SE3StateSpace> start(space);
    start->setX(startPosition[0]);
    start->setY(startPosition[1]);
    start->setZ(startPosition[2]);
    start->rotation().setIdentity();

    ob::ScopedState<ob::SE3StateSpace> goal(space);
    goal->setX(goalPosition[0]);
    goal->setY(goalPosition[1]);
    goal->setZ(goalPosition[2]);
    goal->rotation().setIdentity();

    ss.setStartAndGoalStates(start, goal);

    switch (planner) {
    case SimpleSetup::RRT:
    {
        ob::PlannerPtr planner(new og::RRT(ss.getSpaceInformation()));
        ss.setPlanner(planner);
    }
        break;
    case SimpleSetup::RRTCONNECT:
    {
        ob::PlannerPtr planner(new og::RRTConnect(ss.getSpaceInformation()));
        ss.setPlanner(planner);
    }
        break;
    case SimpleSetup::RRTSTAR:
    {
        ob::PlannerPtr planner(new og::RRTstar(ss.getSpaceInformation()));
        ss.setPlanner(planner);
    }
        break;
    case SimpleSetup::PRRT:
    {
        ob::PlannerPtr planner(new og::pRRT(ss.getSpaceInformation()));
        ss.setPlanner(planner);
    }
        break;
    default:
        break;
    }

    ss.setup();
//    ss.print(mv->cout());

    ob::PlannerStatus solved = ss.solve(calculationTime);

    if(solved) {
        mv->putln(_("Found solution:"));
        isSolved = true;

        og::PathGeometric pathes = ss.getSolutionPath();
        self->postPlannerFunction(pathes);

        ss.simplifySolution();
//        ss.getSolutionPath().print(mv->cout());
    } else {
        mv->putln(_("No solution found"));
        isSolved = false;
    }
}


bool SimpleSetup::isSolved() const
{
    return impl->isSolved;
}


bool SimpleSetup::Impl::isStateValid(const ob::State* state)
{
    const auto *se3state = state->as<ob::SE3StateSpace::StateType>();
    const auto *pos = se3state->as<ob::RealVectorStateSpace::StateType>(0);
    const auto *rot = se3state->as<ob::SO3StateSpace::StateType>(1);

    bool result = self->midPlannerFunction(state);

    return ((const void*)rot != (const void*)pos) && result;
}
