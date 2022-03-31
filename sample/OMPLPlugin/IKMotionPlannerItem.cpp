/**
   \file
   \author Kenta Suzuki
*/

#include "IKMotionPlannerItem.h"
#include <cnoid/Archive>
#include <cnoid/BodyItem>
#include <cnoid/Button>
#include <cnoid/Dialog>
#include <cnoid/EigenArchive>
#include <cnoid/EigenTypes>
#include <cnoid/ItemManager>
#include <cnoid/ItemTreeView>
#include <cnoid/JointPath>
#include <cnoid/MenuManager>
#include <cnoid/MessageView>
#include <cnoid/PointSetItem>
#include <cnoid/PutPropertyFunction>
#include <cnoid/SceneDrawables>
#include <cnoid/Selection>
#include <cnoid/Separator>
#include <cnoid/SpinBox>
#include <cnoid/TimeBar>
#include <cnoid/WorldItem>
#include <fmt/format.h>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QLabel>
#include <QVBoxLayout>
#include <ompl/base/SpaceInformation.h>
#include <ompl/base/spaces/SE3StateSpace.h>
#include <ompl/geometric/planners/rrt/RRT.h>
#include <ompl/geometric/planners/rrt/RRTConnect.h>
#include <ompl/geometric/planners/rrt/RRTstar.h>
#include <ompl/geometric/planners/rrt/pRRT.h>
#include <ompl/geometric/SimpleSetup.h>
#include <ompl/config.h>
#include "sample/SimpleController/Interpolator.h"
#include "gettext.h"

using namespace cnoid;
using namespace std;

namespace ob = ompl::base;
namespace og = ompl::geometric;

namespace cnoid {

class PlannerConfigDialog : public Dialog
{
public:
    PlannerConfigDialog();

    DoubleSpinBox* xminSpin;
    DoubleSpinBox* xmaxSpin;
    DoubleSpinBox* yminSpin;
    DoubleSpinBox* ymaxSpin;
    DoubleSpinBox* zminSpin;
    DoubleSpinBox* zmaxSpin;
};

class IKMotionPlannerItemImpl
{
public:
    IKMotionPlannerItemImpl(IKMotionPlannerItem* self);
    IKMotionPlannerItemImpl(IKMotionPlannerItem* self, const IKMotionPlannerItemImpl& org);
    IKMotionPlannerItem* self;

    PlannerConfigDialog* config;
    BodyItem* bodyItem;
    Link* baseLink;
    Link* endLink;
    MessageView* mv;
    vector<Vector3> solutions;
    Interpolator<VectorXd> interpolator;
    bool isSolved;
    PointSetItem* statePointSetItem;
    PointSetItem* solvedPointSetItem;
    TimeBar* tb;
    string baseLinkName;
    string endLinkName;
    double calculationTime;
    double timeLength;
    Selection plannerType;
    Vector3 startPosition;
    Vector3 goalPosition;

    void onStartTriggered();
    void onGoalTriggered();
    void onGenerateTriggered();
    void planWithSimpleSetup();
    bool isStateValid(const ob::State* state);
    void onPlaybackStarted(const double& time);
    void onTimeChanged(const double& time);
    void onTreePathChanged();
    bool onStartPositionPropertyChanged(const string& text);
    bool onGoalPositionPropertyChanged(const string& text);
    void doPutProperties(PutPropertyFunction& putProperty);
    bool store(Archive& archive);
    bool restore(const Archive& archive);
};

}


IKMotionPlannerItem::IKMotionPlannerItem()
{
    impl = new IKMotionPlannerItemImpl(this);
}


IKMotionPlannerItemImpl::IKMotionPlannerItemImpl(IKMotionPlannerItem* self)
    : self(self),
      mv(MessageView::instance()),
      config(new PlannerConfigDialog),
      tb(TimeBar::instance())
{
    bodyItem = nullptr;
    baseLink = nullptr;
    endLink = nullptr;
    solutions.clear();
    interpolator.clear();
    isSolved = false;
    statePointSetItem = nullptr;
    solvedPointSetItem = nullptr;
    baseLinkName.clear();
    endLinkName.clear();
    calculationTime = 1.0;
    timeLength = 1.0;
    startPosition << 0.0, 0.0, 0.0;
    goalPosition << 0.0, 0.0, 0.0;

    plannerType.setSymbol(IKMotionPlannerItem::RRT, N_("RRT"));
    plannerType.setSymbol(IKMotionPlannerItem::RRTCONNECT, N_("RRTConnect"));
    plannerType.setSymbol(IKMotionPlannerItem::RRTSTAR, N_("RRT*"));
    plannerType.setSymbol(IKMotionPlannerItem::PRRT, N_("pRRT"));

    tb->sigPlaybackStarted().connect([&](double time){ onPlaybackStarted(time); });
    tb->sigTimeChanged().connect([&](double time){ onTimeChanged(time); return true; });
}


IKMotionPlannerItem::IKMotionPlannerItem(const IKMotionPlannerItem& org)
    : Item(org),
      impl(new IKMotionPlannerItemImpl(this, *org.impl))
{

}


IKMotionPlannerItemImpl::IKMotionPlannerItemImpl(IKMotionPlannerItem* self, const IKMotionPlannerItemImpl& org)
    : self(self),
      mv(MessageView::instance()),
      config(new PlannerConfigDialog),
      tb(TimeBar::instance())
{
    baseLinkName = org.baseLinkName;
    endLinkName = org.endLinkName;
    calculationTime = org.calculationTime;
    timeLength = org.timeLength;
    plannerType = org.plannerType;
    startPosition = org.startPosition;
    goalPosition = org.goalPosition;
}


IKMotionPlannerItem::~IKMotionPlannerItem()
{
    delete impl;
}


void IKMotionPlannerItem::initializeClass(ExtensionManager* ext)
{
    string version = OMPL_VERSION;
    MessageView::instance()->putln(fmt::format("OMPL version: {0}", version));

    ext->itemManager()
            .registerClass<IKMotionPlannerItem>(N_("IKMotionPlannerItem"))
            .addCreationPanel<IKMotionPlannerItem>();

    ItemTreeView::instance()->customizeContextMenu<IKMotionPlannerItem>(
        [](IKMotionPlannerItem* item, MenuManager& menuManager, ItemFunctionDispatcher menuFunction) {
            menuManager.setPath("/").setPath(_("IK Motion Planner"));
            menuManager.addItem(_("Set start"))->sigTriggered().connect(
                        [item](){ item->impl->onStartTriggered(); });
            menuManager.addItem(_("Set goal"))->sigTriggered().connect(
                        [item](){ item->impl->onGoalTriggered(); });
            menuManager.addItem(_(_("Generate a path")))->sigTriggered().connect(
                        [item](){ item->impl->onGenerateTriggered(); });
            menuManager.setPath("/");
            menuManager.addItem(_("Configuration of planner"))->sigTriggered().connect(
                        [item](){ item->impl->config->show(); });
            menuManager.setPath("/");
            menuManager.addSeparator();
            menuFunction.dispatchAs<Item>(item);
        });
}


void IKMotionPlannerItemImpl::onStartTriggered()
{
    onTreePathChanged();
    if(endLink) {
        startPosition = endLink->T().translation();
    }
}


void IKMotionPlannerItemImpl::onGoalTriggered()
{
    onTreePathChanged();
    if(endLink) {
        goalPosition = endLink->T().translation();
    }
}


void IKMotionPlannerItemImpl::onGenerateTriggered()
{
    onTreePathChanged();
    if(bodyItem) {
        bodyItem->restoreInitialState(true);
        planWithSimpleSetup();
    }
}


void IKMotionPlannerItemImpl::onPlaybackStarted(const double& time)
{
    if(isSolved) {
        interpolator.clear();
        int numPoints = solutions.size();
        double dt = timeLength / (double)numPoints;

        for(size_t i = 0; i < solutions.size(); ++i) {
            interpolator.appendSample(dt * (double)i, solutions[i]);
        }
        interpolator.update();
    }
}


void IKMotionPlannerItemImpl::onTimeChanged(const double& time)
{
    if(bodyItem && tb->isDoingPlayback()) {
        Body* body = bodyItem->body();
        if(body && baseLink && endLink) {
            auto path = JointPath::getCustomPath(body, baseLink, endLink);
            VectorXd p(6);
            p = interpolator.interpolate(time);
            Isometry3 T;
            T.linear() = endLink->R();
            T.translation() = Vector3(p.head<3>());
            if(path->calcInverseKinematics(T)) {
                bodyItem->notifyKinematicStateChange(true);
            }
        }
    }
}


void IKMotionPlannerItemImpl::planWithSimpleSetup()
{
    auto space(std::make_shared<ob::SE3StateSpace>());

    ob::RealVectorBounds bounds(3);
    bounds.setLow(0, config->xminSpin->value());
    bounds.setHigh(0, config->xmaxSpin->value());
    bounds.setLow(1, config->yminSpin->value());
    bounds.setHigh(1, config->ymaxSpin->value());
    bounds.setLow(2, config->zminSpin->value());
    bounds.setHigh(2, config->zmaxSpin->value());
    space->setBounds(bounds);

    og::SimpleSetup ss(space);

    self->clearChildren();
    statePointSetItem = new PointSetItem;
    statePointSetItem->setName("StatePointSet");
    statePointSetItem->setRenderingMode(PointSetItem::VOXEL);
    statePointSetItem->setVoxelSize(0.03);
    statePointSetItem->setChecked(true);
    self->addSubItem(statePointSetItem);

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
    case IKMotionPlannerItem::RRT:
    {
        ob::PlannerPtr planner(new og::RRT(ss.getSpaceInformation()));
        ss.setPlanner(planner);
    }
        break;
    case IKMotionPlannerItem::RRTCONNECT:
    {
        ob::PlannerPtr planner(new og::RRTConnect(ss.getSpaceInformation()));
        ss.setPlanner(planner);
    }
        break;
    case IKMotionPlannerItem::RRTSTAR:
    {
        ob::PlannerPtr planner(new og::RRTstar(ss.getSpaceInformation()));
        ss.setPlanner(planner);
    }
        break;
    case IKMotionPlannerItem::PRRT:
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
        const int numPoints = pathes.getStateCount();
        solutions.clear();

        solvedPointSetItem = new PointSetItem;
        solvedPointSetItem->setName("SolvedPointSet");
        solvedPointSetItem->setRenderingMode(PointSetItem::VOXEL);
        solvedPointSetItem->setVoxelSize(0.04);
        solvedPointSetItem->setChecked(true);
        self->addSubItem(solvedPointSetItem);

        for(size_t i = 0; i < pathes.getStateCount(); ++i) {
            ob::State* state = pathes.getState(i);
            float x = state->as<ob::SE3StateSpace::StateType>()->getX();
            float y = state->as<ob::SE3StateSpace::StateType>()->getY();
            float z = state->as<ob::SE3StateSpace::StateType>()->getZ();
            solutions.push_back(Vector3(x, y, z));

            if(bodyItem) {
                Body* body = bodyItem->body();
                bodyItem->restoreInitialState(true);
                if(baseLink && endLink) {
                    if(baseLink != endLink) {
                        auto path = JointPath::getCustomPath(body, baseLink, endLink);
                        Isometry3 T;
                        T.linear() = endLink->R();
                        T.translation() = Vector3(x, y, z);
                        if(path->calcInverseKinematics(T)) {
                            bodyItem->notifyKinematicStateChange(true);
                        }
                    }
                }
            }
        }

        {
            SgVertexArray& points = *solvedPointSetItem->pointSet()->getOrCreateVertices();
            SgColorArray& colors = *solvedPointSetItem->pointSet()->getOrCreateColors();
            const int numSolutions = solutions.size();
            points.resize(numSolutions);
            colors.resize(numSolutions);
            for(int i = 0; i < numSolutions; ++i) {
                Vector3f point = Vector3f(solutions[i][0], solutions[i][1], solutions[i][2]);
                points[i] = point;
                Vector3f& c = colors[i];
                c[0] = 0.0;
                c[1] = 1.0;
                c[2] = 0.0;
            }
            solvedPointSetItem->notifyUpdate();
        }

        {
            vector<Vector3> src;
            for(int i = 0; i < statePointSetItem->numAttentionPoints(); ++i) {
                Vector3 point = statePointSetItem->attentionPoint(i);
                src.push_back(point);
            }

            statePointSetItem->clearAttentionPoints();
            SgVertexArray& points = *statePointSetItem->pointSet()->getOrCreateVertices();
            SgColorArray& colors = *statePointSetItem->pointSet()->getOrCreateColors();
            const int numStates = src.size();
            points.resize(numStates);
            colors.resize(numStates);
            for(int i = 0; i < numStates; ++i) {
                Vector3f point = Vector3f(src[i][0], src[i][1], src[i][2]);
                points[i] = point;
                Vector3f& c = colors[i];
                c[0] = 1.0;
                c[1] = 0.0;
                c[2] = 0.0;
            }
            statePointSetItem->notifyUpdate();
        }

        ss.simplifySolution();
//        ss.getSolutionPath().print(mv->cout());
    } else {
        mv->putln(_("No solution found"));
        isSolved = false;
    }
}


bool IKMotionPlannerItemImpl::isStateValid(const ob::State* state)
{
    const auto *se3state = state->as<ob::SE3StateSpace::StateType>();
    const auto *pos = se3state->as<ob::RealVectorStateSpace::StateType>(0);
    const auto *rot = se3state->as<ob::SO3StateSpace::StateType>(1);

    float x = state->as<ob::SE3StateSpace::StateType>()->getX();
    float y = state->as<ob::SE3StateSpace::StateType>()->getY();
    float z = state->as<ob::SE3StateSpace::StateType>()->getZ();

    bool solved = false;
    bool collided = false;
    statePointSetItem->addAttentionPoint(Vector3(x, y, z));

    if(bodyItem) {
        Body* body = bodyItem->body();
        bodyItem->restoreInitialState(true);
        if(baseLink && endLink) {
            if(baseLink != endLink) {
                auto path = JointPath::getCustomPath(body, baseLink, endLink);
                Isometry3 T;
                T.linear() = endLink->R();
                T.translation() = Vector3(x, y, z);
                if(path->calcInverseKinematics(T)) {
                    bodyItem->notifyKinematicStateChange(true);
                    solved = true;
                    WorldItem* worldItem = bodyItem->findOwnerItem<WorldItem>();
                    if(worldItem) {
                        worldItem->updateCollisions();
                        vector<CollisionLinkPairPtr> collisions = bodyItem->collisions();
                        for(size_t i = 0; i < collisions.size(); ++i) {
                            CollisionLinkPairPtr collision = collisions[i];
                            if((collision->body[0] == body) || (collision->body[1] == body)) {
                                if(!collision->isSelfCollision()) {
                                    collided = true;
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    return ((const void*)rot != (const void*)pos) && solved && !collided;
}


Item* IKMotionPlannerItem::doDuplicate() const
{
    return new IKMotionPlannerItem(*this);
}


void IKMotionPlannerItem::onTreePathChanged()
{
    if(parentItem()) {
        impl->onTreePathChanged();
    }
}


void IKMotionPlannerItemImpl::onTreePathChanged()
{
    BodyItem* newBodyItem = self->findOwnerItem<BodyItem>();
    if(newBodyItem != bodyItem) {
        bodyItem = newBodyItem;
        Body* body = bodyItem->body();
        baseLink = body->link(baseLinkName);
        endLink = body->link(endLinkName);
    }
}


bool IKMotionPlannerItemImpl::onStartPositionPropertyChanged(const string& text)
{
    if(toVector3(text, startPosition)) {
        return true;
    }
    return false;
}


bool IKMotionPlannerItemImpl::onGoalPositionPropertyChanged(const string& text)
{
    if(toVector3(text, goalPosition)) {
        return true;
    }
    return false;
}


void IKMotionPlannerItem::doPutProperties(PutPropertyFunction& putProperty)
{
    impl->doPutProperties(putProperty);
}


void IKMotionPlannerItemImpl::doPutProperties(PutPropertyFunction& putProperty)
{
    putProperty(_("Base link"), baseLinkName, changeProperty(baseLinkName));
    putProperty(_("End link"), endLinkName, changeProperty(endLinkName));
    putProperty(_("Geometric planner"), plannerType,
                [&](int index){ return plannerType.select(index); });
    putProperty(_("Start position"), str(startPosition),
                [this](const string& text){ return onStartPositionPropertyChanged(text); });
    putProperty(_("Goal position"), str(goalPosition),
                [this](const string& text){ return onGoalPositionPropertyChanged(text); });
    putProperty(_("Calculation time"), calculationTime, changeProperty(calculationTime));
    putProperty(_("Time length"), timeLength, changeProperty(timeLength));
}


bool IKMotionPlannerItem::store(Archive& archive)
{
    return impl->store(archive);
}


bool IKMotionPlannerItemImpl::store(Archive& archive)
{
    archive.write("base_link", baseLinkName);
    archive.write("end_link", endLinkName);
    archive.write("planner_type", plannerType.which());
    write(archive, "start_position", startPosition);
    write(archive, "goal_position", goalPosition);
    archive.write("calculation_time", calculationTime);
    archive.write("time_length", timeLength);
    return true;
}


bool IKMotionPlannerItem::restore(const Archive& archive)
{
    return impl->restore(archive);
}


bool IKMotionPlannerItemImpl::restore(const Archive& archive)
{
    archive.read("base_link", baseLinkName);
    archive.read("end_link", endLinkName);
    plannerType.select(archive.get("planner_type", 0));
    read(archive, "start_position", startPosition);
    read(archive, "goal_position", goalPosition);
    archive.read("calculation_time", calculationTime);
    archive.read("time_length", timeLength);
    return true;
}


PlannerConfigDialog::PlannerConfigDialog()
{
    setWindowTitle(_("Configuration of planner"));

    xminSpin = new DoubleSpinBox;
    xminSpin->setRange(-1000.0, 0.0);
    xminSpin->setValue(-5.0);
    xmaxSpin = new DoubleSpinBox;
    xmaxSpin->setRange(0.0, 1000.0);
    xmaxSpin->setValue(5.0);
    yminSpin = new DoubleSpinBox;
    yminSpin->setRange(-1000.0, 0.0);
    yminSpin->setValue(-5.0);
    ymaxSpin = new DoubleSpinBox;
    ymaxSpin->setRange(0.0, 1000.0);
    ymaxSpin->setValue(5.0);
    zminSpin = new DoubleSpinBox;
    zminSpin->setRange(-1000.0, 0.0);
    zminSpin->setValue(-5.0);
    zmaxSpin = new DoubleSpinBox;
    zmaxSpin->setRange(0.0, 1000.0);
    zmaxSpin->setValue(5.0);

    QGridLayout* gbox = new QGridLayout;
    gbox->addWidget(new QLabel(_("min")), 0, 1);
    gbox->addWidget(new QLabel(_("max")), 0, 2);
    gbox->addWidget(new QLabel(_("x [m]")), 1, 0);
    gbox->addWidget(xminSpin, 1, 1);
    gbox->addWidget(xmaxSpin, 1, 2);
    gbox->addWidget(new QLabel(_("y [m]")), 2, 0);
    gbox->addWidget(yminSpin, 2, 1);
    gbox->addWidget(ymaxSpin, 2, 2);
    gbox->addWidget(new QLabel(_("z [m]")), 3, 0);
    gbox->addWidget(zminSpin, 3, 1);
    gbox->addWidget(zmaxSpin, 3, 2);

    auto buttonBox = new QDialogButtonBox(this);
    auto okButton = new PushButton(_("&Ok"));
    buttonBox->addButton(okButton, QDialogButtonBox::AcceptRole);
    connect(buttonBox, &QDialogButtonBox::accepted, [this](){ this->accept(); });

    QVBoxLayout* vbox = new QVBoxLayout;
    vbox->addLayout(new HSeparatorBox(new QLabel(_("BoundingBox"))));
    vbox->addLayout(gbox);
    vbox->addWidget(new HSeparator);
    vbox->addWidget(buttonBox);
    setLayout(vbox);
}
