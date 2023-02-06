/**
   \file
   \author Kenta Suzuki
*/

#include "SimpleSetupItem.h"
#include <cnoid/Archive>
#include <cnoid/EigenArchive>
#include <cnoid/ItemManager>
#include <cnoid/MessageView>
#include <cnoid/PutPropertyFunction>
#include <cnoid/SceneDrawables>
#include <cnoid/SceneGraph>
#include <cnoid/Selection>
#include "gettext.h"

using namespace cnoid;
using namespace std;

namespace ob = ompl::base;
namespace og = ompl::geometric;

namespace {

class RegionLocation : public LocationProxy
{
public:
    RegionLocation(SimpleSetupItemImpl* impl);
    SimpleSetupItemImpl* impl;

    Signal<void()> sigLocationChanged_;

    virtual Item* getCorrespondingItem() override;
    virtual Isometry3 getLocation() const override;
    virtual bool setLocation(const Isometry3& T) override;
    virtual SignalProxy<void()> sigLocationChanged() override;
};

}

namespace cnoid {

class SimpleSetupItemImpl
{
public:
    SimpleSetupItemImpl(SimpleSetupItem* self);
    SimpleSetupItemImpl(SimpleSetupItem* self, const SimpleSetupItemImpl& org);
    SimpleSetupItem* self;

    Isometry3 regionOffset;
    SgPosTransformPtr scene;
    Selection plannerType;
    BoundingBox bb;
    Vector3 startPosition;
    Vector3 goalPosition;

    ref_ptr<RegionLocation> regionLocation;

    void createScene();
    void updateScenePosition();
    bool onPlannerPropertyChanged(const int& index);
    bool onBBMinPropertyChanged(const string& str);
    bool onBBMaxPropertyChanged(const string& str);
    bool onStartPositionPropertyChanged(const string& str);
    bool onGoalPositionPropertyChanged(const string& str);
    bool onCalculationTimePropertyChanged(const double& value);
    void doPutProperties(PutPropertyFunction& putProperty);
    bool store(Archive& archive);
    bool restore(const Archive& archive);
};

}


SimpleSetupItem::SimpleSetupItem()
{
    impl = new SimpleSetupItemImpl(this);
}


SimpleSetupItemImpl::SimpleSetupItemImpl(SimpleSetupItem* self)
    : self(self)
{
    plannerType.setSymbol(SimpleSetup::RRT, N_("RRT"));
    plannerType.setSymbol(SimpleSetup::RRTCONNECT, N_("RRTConnect"));
    plannerType.setSymbol(SimpleSetup::RRTSTAR, N_("RRT*"));
    plannerType.setSymbol(SimpleSetup::PRRT, N_("pRRT"));

    Vector3 min(-5.0, -5.0, -5.0);
    Vector3 max(5.0, 5.0, 5.0);
    bb.set(min, max);
    startPosition << 0.0, 0.0, 0.0;
    goalPosition << 0.0, 0.0, 0.0;
    regionOffset.setIdentity();
}


SimpleSetupItem::SimpleSetupItem(const SimpleSetupItem& org)
    : Item(org),
      SimpleSetup(org),
      impl(new SimpleSetupItemImpl(this, *org.impl))
{

}


SimpleSetupItemImpl::SimpleSetupItemImpl(SimpleSetupItem* self, const SimpleSetupItemImpl& org)
    : self(self)
{
    plannerType = org.plannerType;
    bb = org.bb;
    startPosition = org.startPosition;
    goalPosition = org.goalPosition;
    regionOffset = org.regionOffset;
}


SimpleSetupItem::~SimpleSetupItem()
{
    delete impl;
}


void SimpleSetupItem::initializeClass(ExtensionManager* ext)
{
    string version = OMPL_VERSION;
    MessageView::instance()->putln(fmt::format("OMPL version: {0}", version));

    ext->itemManager().registerClass<SimpleSetupItem>(N_("SimpleSetupItem"));
}


void SimpleSetupItem::setRegionOffset(const Isometry3& T)
{
    impl->regionOffset = T;
    // impl->updateScenePosition();
    if(impl->regionLocation) {
        impl->regionLocation->sigLocationChanged_();
    }
}


const Isometry3& SimpleSetupItem::regionOffset() const
{
    return impl->regionOffset;
}


// LocationProxyPtr SimpleSetupItem::getLocationProxy()
// {
//     if(!impl->regionLocation) {
//         impl->regionLocation = new RegionLocation(impl);
//     }
//     return impl->regionLocation;
// }


RegionLocation::RegionLocation(SimpleSetupItemImpl* impl)
    : LocationProxy(GlobalLocation),
      impl(impl)
{

}


Item* RegionLocation::getCorrespondingItem()
{
    return impl->self;
}


Isometry3 RegionLocation::getLocation() const
{
    return impl->regionOffset;
}


bool RegionLocation::setLocation(const Isometry3& T)
{
    impl->self->setRegionOffset(T);
    impl->self->notifyUpdate();
    return true;
}


SignalProxy<void()> RegionLocation::sigLocationChanged()
{
    return sigLocationChanged_;
}


SgNode* SimpleSetupItem::getScene()
{
    if(!impl->scene) {
        impl->createScene();
    }
    return impl->scene;
}


void SimpleSetupItemImpl::createScene()
{
    if(!scene) {
        scene = new SgPosTransform(regionOffset);
    } else {
        scene->clearChildren();
    }

    SgVertexArray* vertices = new SgVertexArray;
    auto& v = *vertices;
    Vector3 min = self->boundingBox().min();
    Vector3 max = self->boundingBox().max();
    v.resize(8);
    v[0] << min[0], min[1], min[2];
    v[1] << min[0], min[1], max[2];
    v[2] << min[0], max[1], min[2];
    v[3] << min[0], max[1], max[2];
    v[4] << max[0], min[1], min[2];
    v[5] << max[0], min[1], max[2];
    v[6] << max[0], max[1], min[2];
    v[7] << max[0], max[1], max[2];
//    vertices->notifyUpdate();

    SgLineSet* lineSet = new SgLineSet;
    lineSet->setVertices(vertices);
    lineSet->setLineWidth(2.0f);
    lineSet->setNumLines(12);
    lineSet->setLine(0, 0, 1);
    lineSet->setLine(1, 1, 3);
    lineSet->setLine(2, 3, 2);
    lineSet->setLine(3, 2, 0);
    lineSet->setLine(4, 4, 5);
    lineSet->setLine(5, 5, 7);
    lineSet->setLine(6, 7, 6);
    lineSet->setLine(7, 6, 4);
    lineSet->setLine(8, 0, 4);
    lineSet->setLine(9, 1, 5);
    lineSet->setLine(10, 3, 7);
    lineSet->setLine(11, 2, 6);

    auto material = lineSet->getOrCreateMaterial();
    material->setDiffuseColor(Vector3f(1.0f, 1.0f, 0.0f));
    scene->addChild(lineSet);
}


void SimpleSetupItemImpl::updateScenePosition()
{
    if(scene) {
        scene->setPosition(regionOffset);
        scene->notifyUpdate();
    }
}


bool SimpleSetupItemImpl::onPlannerPropertyChanged(const int& index)
{
    self->setPlanner(index);
    return plannerType.select(index);
}


bool SimpleSetupItemImpl::onBBMinPropertyChanged(const string& str)
{
    Vector3 min;
    if(toVector3(str, min)) {
        Vector3 max = bb.max();
        bb.set(min, max);
        self->setBoundingBox(bb);
        if(scene) {
            createScene();
            scene->notifyUpdate();
        }
        self->notifyUpdate();
        return true;
    }
    return false;
}


bool SimpleSetupItemImpl::onBBMaxPropertyChanged(const string& str)
{
    Vector3 max;
    if(toVector3(str, max)) {
        Vector3 min = bb.min();
        bb.set(min, max);
        self->setBoundingBox(bb);
        if(scene) {
            createScene();
            scene->notifyUpdate();
        }
        self->notifyUpdate();
        return true;
    }
    return false;
}


bool SimpleSetupItemImpl::onStartPositionPropertyChanged(const string& str)
{
    if(toVector3(str, startPosition)) {
        self->setStartPosition(startPosition);
        return true;
    }
    return false;
}


bool SimpleSetupItemImpl::onGoalPositionPropertyChanged(const string& str)
{
    if(toVector3(str, goalPosition)) {
        self->setGoalPosition(goalPosition);
        return true;
    }
    return false;
}


bool SimpleSetupItemImpl::onCalculationTimePropertyChanged(const double& value)
{
    self->setCalculationTime(value);
    return true;
}


void SimpleSetupItem::prePlannerFunction()
{

}


bool SimpleSetupItem::midPlannerFunction(const ob::State* state)
{
    return true;
}


void SimpleSetupItem::postPlannerFunction(og::PathGeometric& pathes)
{

}


Item* SimpleSetupItem::doDuplicate() const
{
    return new SimpleSetupItem(*this);
}


void SimpleSetupItem::doPutProperties(PutPropertyFunction& putProperty)
{
    impl->doPutProperties(putProperty);
}


void SimpleSetupItemImpl::doPutProperties(PutPropertyFunction& putProperty)
{
    putProperty(_("Geometric planner"), plannerType,
                [&](int index){ return onPlannerPropertyChanged(index); });

    // putProperty(_("Region offset"), str(Vector3(regionOffset.translation())),
    //             [&](const string& str) {
    //                 Vector3 p;
    //                 if(toVector3(str, p)) {
    //                     regionOffset.translation() = p;
    //                     self->setRegionOffset(regionOffset);
    //                     return true;
    //                 }
    //                 return false;
    //             });

    // auto rpy = rpyFromRot(regionOffset.linear());
    // putProperty(_("Region angle"), str(Vector3(rpy)),
    //             [&](const string& str) {
    //                 Vector3 rpy;
    //                 if(toVector3(str, rpy)) {
    //                     regionOffset.linear() = rotFromRpy(Vector3(rpy));
    //                     self->setRegionOffset(regionOffset);
    //                     return true;
    //                 }
    //                 return false;
    //             });

    putProperty(_("BB min"), str(self->boundingBox().min()),
                [this](const string& str){ return onBBMinPropertyChanged(str); });
    putProperty(_("BB max"), str(self->boundingBox().max()),
                [this](const string& str){ return onBBMaxPropertyChanged(str); });
    putProperty(_("Start position"), str(self->startPosition()),
                [this](const string& str){ return onStartPositionPropertyChanged(str); });
    putProperty(_("Goal position"), str(self->goalPosition()),
                [this](const string& str){ return onGoalPositionPropertyChanged(str); });
    putProperty(_("Calculation time"), self->calculationTime(),
                [&](double value){ return onCalculationTimePropertyChanged(value); });
}


bool SimpleSetupItem::store(Archive& archive)
{
    return impl->store(archive);
}


bool SimpleSetupItemImpl::store(Archive& archive)
{
    archive.write("planner_type", plannerType.which());
    if(!regionOffset.translation().isZero()) {
        write(archive, "translation", regionOffset.translation());
    }
    AngleAxis aa(regionOffset.linear());
    if(aa.angle() != 0.0) {
        writeDegreeAngleAxis(archive, "rotation", aa);
    }
    write(archive, "bb_min", self->boundingBox().min());
    write(archive, "bb_max", self->boundingBox().max());
    write(archive, "start_position", self->startPosition());
    write(archive, "goal_position", self->goalPosition());
    archive.write("calculation_time", self->calculationTime());
    return true;
}


bool SimpleSetupItem::restore(const Archive& archive)
{
    return impl->restore(archive);
}


bool SimpleSetupItemImpl::restore(const Archive& archive)
{
    Vector3 p;
    plannerType.select(archive.get("planner_type", 0));
    bool offsetUpdated = false;
    if(read(archive, "translation", p)) {
        regionOffset.translation() = p;
        offsetUpdated = true;
    }
    AngleAxis aa;
    if(readDegreeAngleAxis(archive, "rotation", aa)) {
        regionOffset.linear() = aa.toRotationMatrix();
        offsetUpdated = true;
    }
    if(offsetUpdated) {
        self->setRegionOffset(regionOffset);
    }

    bool bbUpdated = false;
    Vector3 min(-5.0, -5.0, -5.0);
    Vector3 max(5.0, 5.0, 5.0);
    if(read(archive, "bb_min", min)) {
        bbUpdated = true;
    }
    if(read(archive, "bb_max", max)) {
        bbUpdated = true;
    }
    if(bbUpdated) {
        bb.set(min, max);
        self->setBoundingBox(bb);
    }

    if(read(archive, "start_position", startPosition)) {
        self->setStartPosition(startPosition);
    }
    if(read(archive, "goal_position", goalPosition)) {
        self->setGoalPosition(goalPosition);
    }

    self->setPlanner(plannerType.which());
    self->setCalculationTime(archive.get("calculation_time", 0));
    return true;
}
