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

namespace cnoid {

class IKPlannerItemImpl
{
public:
    IKPlannerItemImpl(IKPlannerItem* self);
    IKPlannerItemImpl(IKPlannerItem* self, const IKPlannerItemImpl& org);
    IKPlannerItem* self;

    BodyItem* bodyItem;
    Link* baseLink;
    Link* endLink;
    vector<Vector3> solutions;
    Interpolator<VectorXd> interpolator;
    PointSetItem* statePointSetItem;
    PointSetItem* solvedPointSetItem;
    TimeBar* tb;
    string baseLinkName;
    string endLinkName;
    double timeLength;

    void updateTargetLinks();
    void onStartTriggered();
    void onGoalTriggered();
    void onGenerateTriggered();
    void onPlaybackStarted(const double& time);
    void onTimeChanged(const double& time);
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
    baseLink = nullptr;
    endLink = nullptr;
    solutions.clear();
    interpolator.clear();
    statePointSetItem = nullptr;
    solvedPointSetItem = nullptr;
    baseLinkName.clear();
    endLinkName.clear();
    timeLength = 1.0;

    tb->sigPlaybackStarted().connect([&](double time){ onPlaybackStarted(time); });
    tb->sigTimeChanged().connect([&](double time){ onTimeChanged(time); return true; });
}


IKPlannerItem::IKPlannerItem(const IKPlannerItem& org)
    : MotionPlannerItem(org),
      impl(new IKPlannerItemImpl(this, *org.impl))
{

}


IKPlannerItemImpl::IKPlannerItemImpl(IKPlannerItem* self, const IKPlannerItemImpl& org)
    : self(self),
      tb(TimeBar::instance())
{
    baseLinkName = org.baseLinkName;
    endLinkName = org.endLinkName;
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
        baseLink = body->link(baseLinkName);
        endLink = body->link(endLinkName);
    }
}


void IKPlannerItemImpl::onStartTriggered()
{
    updateTargetLinks();
    if(endLink) {
        self->setStartPosition(endLink->T().translation());
    }
}


void IKPlannerItemImpl::onGoalTriggered()
{
    updateTargetLinks();
    if(endLink) {
        self->setGoalPosition(endLink->T().translation());
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
        interpolator.clear();
        int numPoints = solutions.size();
        double dt = timeLength / (double)numPoints;

        for(size_t i = 0; i < solutions.size(); ++i) {
            interpolator.appendSample(dt * (double)i, solutions[i]);
        }
        interpolator.update();
    }
}


void IKPlannerItemImpl::onTimeChanged(const double& time)
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


void IKPlannerItem::prePlannerFunction()
{
    impl->prePlannerFunction();
}


void IKPlannerItemImpl::prePlannerFunction()
{
    self->clearChildren();
    statePointSetItem = new PointSetItem;
    statePointSetItem->setName("StatePointSet");
    statePointSetItem->setRenderingMode(PointSetItem::VOXEL);
    statePointSetItem->setVoxelSize(0.03);
    statePointSetItem->setChecked(true);
    self->addSubItem(statePointSetItem);
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
    return solved && !collided;
}


void IKPlannerItem::postPlannerFunction(og::PathGeometric& pathes)
{
    impl->postPlannerFunction(pathes);
}


void IKPlannerItemImpl::postPlannerFunction(og::PathGeometric& pathes)
{
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
        updateTargetLinks();
    }
}


void IKPlannerItem::doPutProperties(PutPropertyFunction& putProperty)
{
    MotionPlannerItem::doPutProperties(putProperty);
    impl->doPutProperties(putProperty);
}


void IKPlannerItemImpl::doPutProperties(PutPropertyFunction& putProperty)
{
    putProperty(_("Base link"), baseLinkName, changeProperty(baseLinkName));
    putProperty(_("End link"), endLinkName, changeProperty(endLinkName));
    putProperty(_("Time length"), timeLength, changeProperty(timeLength));
}


bool IKPlannerItem::store(Archive& archive)
{
    MotionPlannerItem::store(archive);
    return impl->store(archive);
}


bool IKPlannerItemImpl::store(Archive& archive)
{
    archive.write("base_link", baseLinkName);
    archive.write("end_link", endLinkName);
    archive.write("time_length", timeLength);
    return true;
}


bool IKPlannerItem::restore(const Archive& archive)
{
    MotionPlannerItem::restore(archive);
    return impl->restore(archive);
}


bool IKPlannerItemImpl::restore(const Archive& archive)
{
    archive.read("base_link", baseLinkName);
    archive.read("end_link", endLinkName);
    archive.read("time_length", timeLength);
    return true;
}
