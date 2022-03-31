/**
   \file
   \author Kenta Suzuki
*/

#include "MotionPlannerItem.h"
#include <cnoid/Archive>
#include <cnoid/EigenArchive>
#include <cnoid/ItemManager>
#include <cnoid/MessageView>
#include <cnoid/PutPropertyFunction>
#include <cnoid/Selection>
#include <fmt/format.h>
#include <ompl/base/SpaceInformation.h>
#include <ompl/base/spaces/SE3StateSpace.h>
#include <ompl/geometric/planners/rrt/RRT.h>
#include <ompl/geometric/planners/rrt/RRTConnect.h>
#include <ompl/geometric/planners/rrt/RRTstar.h>
#include <ompl/geometric/planners/rrt/pRRT.h>
#include <ompl/config.h>
#include "gettext.h"

using namespace cnoid;
using namespace std;

namespace ob = ompl::base;
namespace og = ompl::geometric;

namespace cnoid {

class MotionPlannerItemImpl
{
public:
    MotionPlannerItemImpl(MotionPlannerItem* self);
    MotionPlannerItemImpl(MotionPlannerItem* self, const MotionPlannerItemImpl& org);
    MotionPlannerItem* self;

    enum PlannerType { RRT, RRTCONNECT, RRTSTAR, PRRT, NUM_PLANNERS };

    Selection plannerType;
    Vector3 bbMin;
    Vector3 bbMax;
    Vector3 startPosition;
    Vector3 goalPosition;
    double calculationTime;
    bool isSolved;
    MessageView* mv;

    void planWithSimpleSetup();
    bool isStateValid(const ob::State* state);
    bool onBBMinPropertyChanged(const string& text);
    bool onBBMaxPropertyChanged(const string& text);
    bool onStartPositionPropertyChanged(const string& text);
    bool onGoalPositionPropertyChanged(const string& text);
    void doPutProperties(PutPropertyFunction& putProperty);
    bool store(Archive& archive);
    bool restore(const Archive& archive);
};

}


MotionPlannerItem::MotionPlannerItem()
{
    impl = new MotionPlannerItemImpl(this);
}


MotionPlannerItemImpl::MotionPlannerItemImpl(MotionPlannerItem* self)
    : self(self),
      mv(MessageView::instance())
{
    plannerType.setSymbol(RRT, N_("RRT"));
    plannerType.setSymbol(RRTCONNECT, N_("RRTConnect"));
    plannerType.setSymbol(RRTSTAR, N_("RRT*"));
    plannerType.setSymbol(PRRT, N_("pRRT"));
    bbMin << -5.0, -5.0, -5.0;
    bbMax << 5.0, 5.0, 5.0;
    startPosition << 0.0, 0.0, 0.0;
    goalPosition << 0.0, 0.0, 0.0;
    calculationTime = 1.0;
    isSolved = false;
}


MotionPlannerItem::MotionPlannerItem(const MotionPlannerItem& org)
    : Item(org),
      impl(new MotionPlannerItemImpl(this, *org.impl))
{

}


MotionPlannerItemImpl::MotionPlannerItemImpl(MotionPlannerItem* self, const MotionPlannerItemImpl& org)
    : self(self),
      mv(MessageView::instance())
{
    plannerType = org.plannerType;
    bbMin = org.bbMin;
    bbMax = org.bbMax;
    startPosition = org.startPosition;
    goalPosition = org.goalPosition;
    calculationTime = org.calculationTime;
    isSolved = org.isSolved;
}


MotionPlannerItem::~MotionPlannerItem()
{
    delete impl;
}


void MotionPlannerItem::initializeClass(ExtensionManager* ext)
{
    string version = OMPL_VERSION;
    MessageView::instance()->putln(fmt::format("OMPL version: {0}", version));

    ext->itemManager().registerClass<MotionPlannerItem>(N_("MotionPlannerItem"));
}


void MotionPlannerItem::setStartPosition(const Vector3& startPosition)
{
    impl->startPosition = startPosition;
}


void MotionPlannerItem::setGoalPosition(const Vector3& goalPosition)
{
    impl->goalPosition = goalPosition;
}


void MotionPlannerItem::planWithSimpleSetup()
{
    impl->planWithSimpleSetup();
}


void MotionPlannerItemImpl::planWithSimpleSetup()
{
    self->prePlannerFunction();

    auto space(std::make_shared<ob::SE3StateSpace>());

    ob::RealVectorBounds bounds(3);
    bounds.setLow(0, bbMin[0]);
    bounds.setHigh(0, bbMax[0]);
    bounds.setLow(1, bbMin[1]);
    bounds.setHigh(1, bbMax[1]);
    bounds.setLow(2, bbMin[2]);
    bounds.setHigh(2, bbMax[2]);
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

    switch (plannerType.which()) {
    case RRT:
    {
        ob::PlannerPtr planner(new og::RRT(ss.getSpaceInformation()));
        ss.setPlanner(planner);
    }
        break;
    case RRTCONNECT:
    {
        ob::PlannerPtr planner(new og::RRTConnect(ss.getSpaceInformation()));
        ss.setPlanner(planner);
    }
        break;
    case RRTSTAR:
    {
        ob::PlannerPtr planner(new og::RRTstar(ss.getSpaceInformation()));
        ss.setPlanner(planner);
    }
        break;
    case PRRT:
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


bool MotionPlannerItem::isSolved() const
{
    return impl->isSolved;
}


bool MotionPlannerItemImpl::isStateValid(const ob::State* state)
{
    const auto *se3state = state->as<ob::SE3StateSpace::StateType>();
    const auto *pos = se3state->as<ob::RealVectorStateSpace::StateType>(0);
    const auto *rot = se3state->as<ob::SO3StateSpace::StateType>(1);

    bool result = self->midPlannerFunction(state);

    return ((const void*)rot != (const void*)pos) && result;
}


bool MotionPlannerItemImpl::onBBMinPropertyChanged(const string& text)
{
    if(toVector3(text, bbMin)) {
        return true;
    }
    return false;
}


bool MotionPlannerItemImpl::onBBMaxPropertyChanged(const string& text)
{
    if(toVector3(text, bbMax)) {
        return true;
    }
    return false;
}


bool MotionPlannerItemImpl::onStartPositionPropertyChanged(const string& text)
{
    if(toVector3(text, startPosition)) {
        return true;
    }
    return false;
}


bool MotionPlannerItemImpl::onGoalPositionPropertyChanged(const string& text)
{
    if(toVector3(text, goalPosition)) {
        return true;
    }
    return false;
}


void MotionPlannerItem::prePlannerFunction()
{

}


bool MotionPlannerItem::midPlannerFunction(const ompl::base::State* state)
{
    return true;
}


void MotionPlannerItem::postPlannerFunction(ompl::geometric::PathGeometric& pathes)
{

}


Item* MotionPlannerItem::doDuplicate() const
{
    return new MotionPlannerItem(*this);
}


void MotionPlannerItem::doPutProperties(PutPropertyFunction& putProperty)
{
    impl->doPutProperties(putProperty);
}


void MotionPlannerItemImpl::doPutProperties(PutPropertyFunction& putProperty)
{
    putProperty(_("Geometric planner"), plannerType,
                [&](int index){ return plannerType.select(index); });
    putProperty(_("BB min"), str(bbMin),
                [this](const string& text){ return onBBMinPropertyChanged(text); });
    putProperty(_("BB max"), str(bbMax),
                [this](const string& text){ return onBBMaxPropertyChanged(text); });
    putProperty(_("Start position"), str(startPosition),
                [this](const string& text){ return onStartPositionPropertyChanged(text); });
    putProperty(_("Goal position"), str(goalPosition),
                [this](const string& text){ return onGoalPositionPropertyChanged(text); });
    putProperty(_("Calculation time"), calculationTime, changeProperty(calculationTime));
}


bool MotionPlannerItem::store(Archive& archive)
{
    return impl->store(archive);
}


bool MotionPlannerItemImpl::store(Archive& archive)
{
    archive.write("planner_type", plannerType.which());
    write(archive, "bb_min", bbMin);
    write(archive, "bb_max", bbMax);
    write(archive, "start_position", startPosition);
    write(archive, "goal_position", goalPosition);
    archive.write("calculation_time", calculationTime);
    return true;
}


bool MotionPlannerItem::restore(const Archive& archive)
{
    return impl->restore(archive);
}


bool MotionPlannerItemImpl::restore(const Archive& archive)
{
    plannerType.select(archive.get("planner_type", 0));
    read(archive, "bb_min", bbMin);
    read(archive, "bb_max", bbMax);
    read(archive, "start_position", startPosition);
    read(archive, "goal_position", goalPosition);
    archive.read("calculation_time", calculationTime);
    return true;
}
