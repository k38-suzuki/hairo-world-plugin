/**
   \file
   \author Kenta Suzuki
*/

#include "IKPlannerItem.h"
#include <cnoid/Archive>
#include <cnoid/BodyItem>
#include <cnoid/EigenArchive>
#include <cnoid/EigenTypes>
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
#include "sample/SimpleController/Interpolator.h"
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

class IKPlannerItemImpl
{
public:
    IKPlannerItemImpl(IKPlannerItem* self);
    IKPlannerItemImpl(IKPlannerItem* self, const IKPlannerItemImpl& org);
    IKPlannerItem* self;

    BodyItem* bodyItem;
    Link* base;
    Link* wrist;
    Interpolator<VectorXd> interpolator;
    vector<Vector3> states;
    vector<Vector3> solutions;
    TimeBar* tb;
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
    void doPutProperties(PutPropertyFunction& putProperty);
    bool store(Archive& archive);
    bool restore(const Archive& archive);
};

}


IKPlannerItem::IKPlannerItem()
{
    impl = new IKPlannerItemImpl(this);
}


IKPlannerItemImpl::IKPlannerItemImpl(IKPlannerItem* self)
    : self(self),
      tb(TimeBar::instance())
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

    tb->sigPlaybackStarted().connect([&](double time){ onPlaybackStarted(time); });
    tb->sigTimeChanged().connect([&](double time){ return onTimeChanged(time); });
}


IKPlannerItem::IKPlannerItem(const IKPlannerItem& org)
    : SimpleSetupItem(org),
      impl(new IKPlannerItemImpl(this, *org.impl))
{

}


IKPlannerItemImpl::IKPlannerItemImpl(IKPlannerItem* self, const IKPlannerItemImpl& org)
    : self(self),
      tb(TimeBar::instance())
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
            .registerClass<IKPlannerItem>(N_("IKPlannerItem"))
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


void IKPlannerItemImpl::updateTargetLinks()
{
    if(bodyItem) {
        Body* body = bodyItem->body();
        base = body->link(baseName);
        wrist = body->link(wristName);
    }
}


void IKPlannerItemImpl::onStartTriggered()
{
    updateTargetLinks();
    if(wrist) {
        self->setStartPosition(wrist->T().translation());
    }
}


void IKPlannerItemImpl::onGoalTriggered()
{
    updateTargetLinks();
    if(wrist) {
        self->setGoalPosition(wrist->T().translation());
    }
}


void IKPlannerItemImpl::onGenerateTriggered()
{
    updateTargetLinks();
    if(bodyItem) {
        bodyItem->restoreInitialState(true);
        self->planWithSimpleSetup();
    }
}


void IKPlannerItemImpl::onPlaybackStarted(const double& time)
{
    if(self->isSolved()) {
        if(bodyItem) {
            bodyItem->restoreInitialState(true);

            interpolator.clear();
            int numPoints = solutions.size();
            double dt = timeLength / (double)numPoints;
            for(size_t i = 0; i < solutions.size(); ++i) {
                interpolator.appendSample(dt * (double)i, solutions[i]);
            }
            interpolator.update();
        }
    }
}


bool IKPlannerItemImpl::onTimeChanged(const double& time)
{
    if(self->isSolved()) {
        if(tb->isDoingPlayback()) {
            VectorXd p(6);
            p = interpolator.interpolate(time);
            calcInverseKinematics(Vector3(p.head<3>()));
            if(time > timeLength) {
                tb->stopPlayback(true);
            }
        }
    }
    return true;
}


bool IKPlannerItemImpl::calcInverseKinematics(const Vector3& position)
{
    if(bodyItem && base && wrist) {
        if(base != wrist) {
            Body* body = bodyItem->body();
            shared_ptr<JointPath> baseToWrist = JointPath::getCustomPath(body, base, wrist);
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


void IKPlannerItemImpl::prePlannerFunction()
{
    self->clearChildren();
    states.clear();
    solutions.clear();
}


bool IKPlannerItem::midPlannerFunction(const ob::State* state)
{
    return impl->midPlannerFunction(state);
}


bool IKPlannerItemImpl::midPlannerFunction(const ob::State* state)
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
                for(size_t i = 0; i < collisions.size(); ++i) {
                    CollisionLinkPairPtr collision = collisions[i];
                    if((collision->body[0] == body) || (collision->body[1] == body)) {
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


void IKPlannerItemImpl::postPlannerFunction(og::PathGeometric& pathes)
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
//    statePointSetItem->setChecked(true);
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


Item* IKPlannerItem::doDuplicate() const
{
    return new IKPlannerItem(*this);
}


void IKPlannerItem::onTreePathChanged()
{
    if(parentItem()) {
        impl->onTreePathChanged();
    }
}


void IKPlannerItemImpl::onTreePathChanged()
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
    impl->doPutProperties(putProperty);
}


void IKPlannerItemImpl::doPutProperties(PutPropertyFunction& putProperty)
{
    putProperty(_("Base link"), baseName, changeProperty(baseName));
    putProperty(_("End link"), wristName, changeProperty(wristName));
    putProperty(_("Time length"), timeLength, changeProperty(timeLength));
}


bool IKPlannerItem::store(Archive& archive)
{
    SimpleSetupItem::store(archive);
    return impl->store(archive);
}


bool IKPlannerItemImpl::store(Archive& archive)
{
    archive.write("base_name", baseName);
    archive.write("wrist_name", wristName);
    archive.write("time_length", timeLength);
    return true;
}


bool IKPlannerItem::restore(const Archive& archive)
{
    SimpleSetupItem::restore(archive);
    return impl->restore(archive);
}


bool IKPlannerItemImpl::restore(const Archive& archive)
{
    archive.read("base_name", baseName);
    archive.read("wrist_name", wristName);
    archive.read("time_length", timeLength);
    return true;
}