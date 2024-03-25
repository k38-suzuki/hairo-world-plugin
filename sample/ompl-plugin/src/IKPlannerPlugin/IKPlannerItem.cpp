/**
   @author Kenta Suzuki
*/

#include "IKPlannerItem.h"
#include <cnoid/Archive>
#include <cnoid/BodyItem>
#include <cnoid/EigenArchive>
#include <cnoid/EigenTypes>
#include <cnoid/ExtensionManager>
#include <cnoid/ItemManager>
#include <cnoid/ItemTreeView>
#include <cnoid/JointPath>
#include <cnoid/MenuManager>
#include <cnoid/PointSetItem>
#include <cnoid/PutPropertyFunction>
#include <cnoid/SceneDrawables>
#include <cnoid/TimeBar>
#include <cnoid/WorldItem>
#include <fmt/format.h>
#include <ompl/base/spaces/SE3StateSpace.h>
#include <sample/SimpleController/Interpolator.h>
#include "gettext.h"

using namespace cnoid;
using namespace std;

namespace ob = ompl::base;
namespace og = ompl::geometric;

namespace {

void updatePointSet(PointSetItem* pointSetItem, const vector<Vector3>& src, const Vector3f& color)
{
    SgVertexArray& points = *pointSetItem->pointSet()->getOrCreateVertices();
    SgColorArray& colors = *pointSetItem->pointSet()->getOrCreateColors();
    const int numSolutions = src.size();
    points.resize(numSolutions);
    colors.resize(numSolutions);
    for(int i = 0; i < numSolutions; ++i) {
        points[i] = Vector3f(src[i][0], src[i][1], src[i][2]);
        Vector3f& c = colors[i];
        c[0] = color[0];
        c[1] = color[1];
        c[2] = color[2];
    }
    pointSetItem->notifyUpdate();
}

}

namespace cnoid {

class IKPlannerItem::Impl
{
public:
    IKPlannerItem* self;

    Impl(IKPlannerItem* self);
    Impl(IKPlannerItem* self, const Impl& org);

    typedef shared_ptr<CollisionLinkPair> CollisionLinkPairPtr;

    BodyItem* bodyItem;
    Link* base;
    Link* wrist;
    Interpolator<VectorXd> interpolator;
    vector<Vector3> states;
    vector<Vector3> solutions;
    string baseName;
    string wristName;
    double timeLength;

    void updateTargetLinks();
    void onStartTriggered();
    void onGoalTriggered();
    void onGenerateTriggered();
    void onPlaybackStarted(const double& time);
    bool onTimeChanged(const double& time);
    bool calcInverseKinematics(const Vector3& position);
    void prePlannerFunction();
    bool midPlannerFunction(const ob::State* state);
    void postPlannerFunction(og::PathGeometric& pathes);
    void onTreePathChanged();
};

}


IKPlannerItem::IKPlannerItem()
{
    impl = new Impl(this);
}


IKPlannerItem::Impl::Impl(IKPlannerItem* self)
    : self(self)
{
    bodyItem = nullptr;
    base = nullptr;
    wrist = nullptr;
    interpolator.clear();
    states.clear();
    solutions.clear();
    baseName.clear();
    wristName.clear();
    timeLength = 1.0;

    TimeBar* timeBar = TimeBar::instance();
    timeBar->sigPlaybackStarted().connect([&](double time){ onPlaybackStarted(time); });
    timeBar->sigTimeChanged().connect([&](double time){ return onTimeChanged(time); });
}


IKPlannerItem::IKPlannerItem(const IKPlannerItem& org)
    : SimpleSetupItem(org),
      impl(new Impl(this, *org.impl))
{

}


IKPlannerItem::Impl::Impl(IKPlannerItem* self, const Impl& org)
    : self(self)
{
    baseName = org.baseName;
    wristName = org.wristName;
    timeLength = org.timeLength;
}


IKPlannerItem::~IKPlannerItem()
{
    delete impl;
}


void IKPlannerItem::initializeClass(ExtensionManager* ext)
{
    ext->itemManager()
        .registerClass<IKPlannerItem, SubSimulatorItem>(N_("IKPlannerItem"))
        .addCreationPanel<IKPlannerItem>();

    ItemTreeView::instance()->customizeContextMenu<IKPlannerItem>(
        [](IKPlannerItem* item, MenuManager& menuManager, ItemFunctionDispatcher menuFunction) {
            menuManager.setPath("/").setPath(_("IK Planner"));
            menuManager.addItem(_("Set start"))->sigTriggered().connect(
                        [item](){ item->impl->onStartTriggered(); });
            menuManager.addItem(_("Set goal"))->sigTriggered().connect(
                        [item](){ item->impl->onGoalTriggered(); });
            menuManager.addItem(_(_("Generate a path")))->sigTriggered().connect(
                        [item](){ item->impl->onGenerateTriggered(); });
            menuManager.setPath("/");
            menuManager.addSeparator();
            menuFunction.dispatchAs<Item>(item);
        });
}


void IKPlannerItem::Impl::updateTargetLinks()
{
    if(bodyItem) {
        Body* body = bodyItem->body();
        base = body->link(baseName);
        wrist = body->link(wristName);
    }
}


void IKPlannerItem::Impl::onStartTriggered()
{
    updateTargetLinks();
    if(wrist) {
        self->setStartPosition(wrist->T().translation());
    }
}


void IKPlannerItem::Impl::onGoalTriggered()
{
    updateTargetLinks();
    if(wrist) {
        self->setGoalPosition(wrist->T().translation());
    }
}


void IKPlannerItem::Impl::onGenerateTriggered()
{
    updateTargetLinks();
    if(bodyItem) {
        bodyItem->restoreInitialState(true);
        self->planWithSimpleSetup();
    }
}


void IKPlannerItem::Impl::onPlaybackStarted(const double& time)
{
    if(self->isSolved()) {
        if(bodyItem) {
            bodyItem->restoreInitialState(true);

            interpolator.clear();
            int numPoints = solutions.size();
            double dt = timeLength / (double)numPoints;
            for(auto& solution : solutions) {
                interpolator.appendSample(dt * (double)i, solution);
            }
            interpolator.update();
        }
    }
}


bool IKPlannerItem::Impl::onTimeChanged(const double& time)
{
    if(self->isSolved()) {
        TimeBar* timeBar = TimeBar::instance();
        if(timeBar->isDoingPlayback()) {
            VectorXd p(6);
            p = interpolator.interpolate(time);
            calcInverseKinematics(Vector3(p.head<3>()));
            if(time > timeLength) {
                timeBar->stopPlayback(true);
            }
        }
    }
    return true;
}


bool IKPlannerItem::Impl::calcInverseKinematics(const Vector3& position)
{
    if(bodyItem && base && wrist) {
        if(base != wrist) {
            Body* body = bodyItem->body();
            shared_ptr<JointPath> baseToWrist = JointPath::getCustomPath(base, wrist);
            Isometry3 T;
            T.linear() = wrist->R();
            T.translation() = position;
            if(baseToWrist->calcInverseKinematics(T)) {
                bodyItem->notifyKinematicStateChange(true);
                return true;
            }
        }
    }
    return false;
}


void IKPlannerItem::prePlannerFunction()
{
    impl->prePlannerFunction();
}


void IKPlannerItem::Impl::prePlannerFunction()
{
    self->clearChildren();
    states.clear();
    solutions.clear();
}


bool IKPlannerItem::midPlannerFunction(const ob::State* state)
{
    return impl->midPlannerFunction(state);
}


bool IKPlannerItem::Impl::midPlannerFunction(const ob::State* state)
{
    float x = state->as<ob::SE3StateSpace::StateType>()->getX();
    float y = state->as<ob::SE3StateSpace::StateType>()->getY();
    float z = state->as<ob::SE3StateSpace::StateType>()->getZ();

    bool result = false;
    states.push_back(Vector3(x, y, z));

    if(calcInverseKinematics(Vector3(x, y, z))) {
        result = true;
        if(bodyItem) {
            Body* body = bodyItem->body();
            WorldItem* worldItem = bodyItem->findOwnerItem<WorldItem>();
            if(worldItem) {
                worldItem->updateCollisions();
                vector<CollisionLinkPairPtr> collisions = bodyItem->collisions();
                for(auto& collision : collisions) {
                    if((collision->link(0)->body() == body) || (collision->link(1)->body() == body)) {
                        if(!collision->isSelfCollision()) {
                            return false;
                        }
                    }
                }
            }
        }
    }
    return result;
}


void IKPlannerItem::postPlannerFunction(og::PathGeometric& pathes)
{
    impl->postPlannerFunction(pathes);
}


void IKPlannerItem::Impl::postPlannerFunction(og::PathGeometric& pathes)
{
    for(size_t i = 0; i < pathes.getStateCount(); ++i) {
        ob::State* state = pathes.getState(i);
        float x = state->as<ob::SE3StateSpace::StateType>()->getX();
        float y = state->as<ob::SE3StateSpace::StateType>()->getY();
        float z = state->as<ob::SE3StateSpace::StateType>()->getZ();
        solutions.push_back(Vector3(x, y, z));
    }
    solutions.push_back(self->goalPosition());

    PointSetItem* statePointSetItem = new PointSetItem;
    statePointSetItem->setName("StatePointSet");
    statePointSetItem->setChecked(true);
    self->addSubItem(statePointSetItem);
    updatePointSet(statePointSetItem, states, Vector3f(1.0, 0.0, 0.0));

    PointSetItem* solvedPointSetItem = new PointSetItem;
    solvedPointSetItem->setName("SolvedPointSet");
    solvedPointSetItem->setRenderingMode(PointSetItem::VOXEL);
    solvedPointSetItem->setVoxelSize(0.01);
    solvedPointSetItem->setChecked(true);
    self->addSubItem(solvedPointSetItem);
    updatePointSet(solvedPointSetItem, solutions, Vector3f(0.0, 1.0, 0.0));
}


Item* IKPlannerItem::doCloneItem(CloneMap* cloneMap) const
{
    return new IKPlannerItem(*this);
}


void IKPlannerItem::onTreePathChanged()
{
    if(parentItem()) {
        impl->onTreePathChanged();
    }
}


void IKPlannerItem::Impl::onTreePathChanged()
{
    BodyItem* newBodyItem = self->findOwnerItem<BodyItem>();
    if(newBodyItem != bodyItem) {
        bodyItem = newBodyItem;
    }
    updateTargetLinks();
}


void IKPlannerItem::doPutProperties(PutPropertyFunction& putProperty)
{
    SimpleSetupItem::doPutProperties(putProperty);
    putProperty(_("Base link"), impl->baseName, changeProperty(impl->baseName));
    putProperty(_("End link"), impl->wristName, changeProperty(impl->wristName));
    putProperty(_("Time length"), impl->timeLength, changeProperty(impl->timeLength));
}


bool IKPlannerItem::store(Archive& archive)
{
    SimpleSetupItem::store(archive);
    archive.write("base_name", impl->baseName);
    archive.write("wrist_name", impl->wristName);
    archive.write("time_length", impl->timeLength);
    return true;
}


bool IKPlannerItem::restore(const Archive& archive)
{
    SimpleSetupItem::restore(archive);
    archive.read("base_name", impl->baseName);
    archive.read("wrist_name", impl->wristName);
    archive.read("time_length", impl->timeLength);
    return true;
}
