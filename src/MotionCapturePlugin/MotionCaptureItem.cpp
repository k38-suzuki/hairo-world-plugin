/**
   @author Kenta Suzuki
*/

#include "MotionCaptureItem.h"
#include <cnoid/Archive>
#include <cnoid/Body>
#include <cnoid/DeviceList>
#include <cnoid/ItemManager>
#include <cnoid/MultiPointSetItem>
#include <cnoid/MultiSE3SeqItem>
#include <cnoid/PointSetItem>
#include <cnoid/PutPropertyFunction>
#include <cnoid/SimulatorItem>
#include <cnoid/UTF8>
#include <cnoid/stdx/filesystem>
#include <vector>
#include "PassiveMarker.h"
#include "LoggerUtil.h"
#include "gettext.h"

using namespace std;
using namespace cnoid;
namespace filesystem = cnoid::stdx::filesystem;

namespace cnoid {

class MotionCaptureItem::Impl
{
public:
    MotionCaptureItem* self;

    Impl(MotionCaptureItem* self);
    Impl(MotionCaptureItem* self, const Impl& org);

    DeviceList<PassiveMarker> markers;
    ItemList<PointSetItem> pointSetItems;
    MultiPointSetItemPtr multiPointSetItem;
    MultiSE3SeqItemPtr motionSeqItem;

    bool initializeSimulation(SimulatorItem* simulatorItem);
    void finalizeSimulation();
    void onPreDynamics();
};

}


MotionCaptureItem::MotionCaptureItem()
    : SubSimulatorItem()
{
    impl = new Impl(this);
}


MotionCaptureItem::Impl::Impl(MotionCaptureItem* self)
    : self(self)
{
    markers.clear();
    pointSetItems.clear();
    multiPointSetItem = nullptr;
    motionSeqItem = nullptr;
}


MotionCaptureItem::MotionCaptureItem(const MotionCaptureItem& org)
    : SubSimulatorItem(org),
      impl(new Impl(this, *org.impl))
{

}


MotionCaptureItem::Impl::Impl(MotionCaptureItem *self, const Impl& org)
    : self(self)
{

}


MotionCaptureItem::~MotionCaptureItem()
{
    delete impl;
}


void MotionCaptureItem::initializeClass(ExtensionManager* ext)
{
    ext->itemManager()
        .registerClass<MotionCaptureItem, SubSimulatorItem>(N_("MotionCaptureItem"))
        .addCreationPanel<MotionCaptureItem>();
}


bool MotionCaptureItem::initializeSimulation(SimulatorItem* simulatorItem)
{
    return impl->initializeSimulation(simulatorItem);
}


bool MotionCaptureItem::Impl::initializeSimulation(SimulatorItem* simulatorItem)
{
    markers.clear();

    for(auto& item : pointSetItems) {
        // item->removeFromParentItem();
    }
    pointSetItems.clear();

    if(multiPointSetItem) {
        multiPointSetItem->setChecked(false);
    }

    const vector<SimulationBody*>& simBodies = simulatorItem->simulationBodies();
    for(auto& simBody : simBodies) {
        Body* body = simBody->body();
        markers << body->devices();
    }

    if(markers.size()) {
        multiPointSetItem = new MultiPointSetItem;
        multiPointSetItem->setName("Motion");
        multiPointSetItem->setChecked(false);
        self->addSubItem(multiPointSetItem);

        for(auto& marker : markers) {
            PointSetItem* pointSetItem = new PointSetItem;
            pointSetItem->setName(marker->name());
            pointSetItem->setChecked(false);
            multiPointSetItem->addSubItem(pointSetItem);
        }
        pointSetItems = multiPointSetItem->descendantItems();

        motionSeqItem = new MultiSE3SeqItem;
        motionSeqItem->setName("MotionSeq");
        multiPointSetItem->addSubItem(motionSeqItem);

        int numParts = markers.size();
        shared_ptr<MultiSE3Seq> log = motionSeqItem->seq();
        log->setNumFrames(0);
        log->setNumParts(numParts);
        log->setFrameRate(1.0 / simulatorItem->worldTimeStep());
        log->setOffsetTime(0.0);

        simulatorItem->addPreDynamicsFunction([&](){ onPreDynamics(); });
    }

    return true;
}


void MotionCaptureItem::finalizeSimulation()
{
    impl->finalizeSimulation();
}


void MotionCaptureItem::Impl::finalizeSimulation()
{
    if(multiPointSetItem) {
        string suffix = getCurrentTimeSuffix();
        filesystem::path mocapDirPath(fromUTF8(mkdir(StandardPath::Downloads, "mocap")));

        multiPointSetItem->setChecked(true);

        for(size_t i = 0; i < markers.size(); ++i) {
            PassiveMarker* marker = markers[i];

            PointSetItem* pointSetItem = pointSetItems[i];
            auto pointSet_ = pointSetItem->pointSet();

            vector<Vector3> src;
            for(int j = 0; j < pointSetItem->numAttentionPoints(); ++j) {
                src.push_back(pointSetItem->attentionPoint(j));
            }
            SgVertexArray& points = *pointSetItem->pointSet()->getOrCreateVertices();
            const int numPoints = src.size();
            points.resize(numPoints);
            for(int j = 0; j < numPoints; ++j) {
                points[j] = Vector3f(src[j][0], src[j][1], src[j][2]);
            }

            SgColorArray& colors = *pointSetItem->pointSet()->getOrCreateColors();
            const int n = numPoints;
            colors.resize(n);
            for(int j = 0; j < n; ++j) {
                Vector3f& c = colors[j];
                c[0] = marker->color()[0];
                c[1] = marker->color()[1];
                c[2] = marker->color()[2];
            }
            pointSet_->notifyUpdate();

            pointSetItem->clearAttentionPoints();
            pointSetItem->notifyUpdate();

            string filename = toUTF8((mocapDirPath / filesystem::path(fromUTF8(pointSetItem->name() + suffix + ".pcd"))).string());
            // pointSetItem->save(filename);
        }

        string filename0 = toUTF8((mocapDirPath / filesystem::path(fromUTF8(multiPointSetItem->name() + suffix + ".yaml"))).string());
        // multiPointSetItem->save(filename0);
    }
}


void MotionCaptureItem::Impl::onPreDynamics()
{
    shared_ptr<MultiSE3Seq> log = motionSeqItem->seq();
    auto frame = log->appendFrame();
    for(size_t i = 0; i < markers.size(); ++i) {
        PassiveMarker* marker = markers[i];
        if(marker->on()) {
            Link* link = marker->link();
            Vector3 p = link->T() * marker->p_local();
            Matrix3 R = link->R() * marker->R_local();
            frame[i].set(p, R);
            pointSetItems[i]->addAttentionPoint(p);
        }
    }
}


Item* MotionCaptureItem::doCloneItem(CloneMap* cloneMap) const
{
    return new MotionCaptureItem(*this);
}


void MotionCaptureItem::doPutProperties(PutPropertyFunction& putProperty)
{
    SubSimulatorItem::doPutProperties(putProperty);
}


bool MotionCaptureItem::store(Archive& archive)
{
    if(!SubSimulatorItem::store(archive)) {
        return false;
    }
    return true;
}


bool MotionCaptureItem::restore(const Archive& archive)
{
    if(!SubSimulatorItem::restore(archive)) {
        return false;
    }
    return true;
}