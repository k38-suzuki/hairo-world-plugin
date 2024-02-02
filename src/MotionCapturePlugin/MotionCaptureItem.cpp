/**
   @author Kenta Suzuki
*/

#include "MotionCaptureItem.h"
#include <cnoid/Body>
#include <cnoid/DeviceList>
#include <cnoid/ExecutablePath>
#include <cnoid/ItemManager>
#include <cnoid/MultiPointSetItem>
#include <cnoid/MultiSE3SeqItem>
#include <cnoid/PointSetItem>
#include <cnoid/PutPropertyFunction>
#include <cnoid/SimulatorItem>
#include <cnoid/stdx/filesystem>
#include <cnoid/UTF8>
#include <QDateTime>
#include <vector>
#include "PassiveMarker.h"
#include "gettext.h"

using namespace cnoid;
using namespace std;
namespace filesystem = cnoid::stdx::filesystem;

namespace cnoid {

class MotionCaptureItemImpl
{
public:
    MotionCaptureItemImpl(MotionCaptureItem* self);
    MotionCaptureItemImpl(MotionCaptureItem* self, const MotionCaptureItemImpl& org);
    MotionCaptureItem* self;

    DeviceList<PassiveMarker> markers;
    vector<PointSetItem*> pointSetItems;
    MultiPointSetItemPtr multiPointSetItem;
    SimulatorItem* simulatorItem;
    MultiSE3SeqItemPtr motionSeqItem;
    bool isMotionDataRecordingEnabled;

    bool initializeSimulation(SimulatorItem* simulatorItem);
    void finalizeSimulation();
    void onPreDynamicsFunction();
    void doPutProperties(PutPropertyFunction& putProperty);
    bool store(Archive& archive);
    bool restore(const Archive& archive);
};

}


MotionCaptureItem::MotionCaptureItem()
{
    impl = new MotionCaptureItemImpl(this);
}


MotionCaptureItemImpl::MotionCaptureItemImpl(MotionCaptureItem* self)
    : self(self)
{
    markers.clear();
    pointSetItems.clear();
    multiPointSetItem = nullptr;
    simulatorItem = nullptr;
    motionSeqItem = nullptr;
    isMotionDataRecordingEnabled = true;
}


MotionCaptureItem::MotionCaptureItem(const MotionCaptureItem& org)
    : SubSimulatorItem(org),
      impl(new MotionCaptureItemImpl(this, *org.impl))
{

}


MotionCaptureItemImpl::MotionCaptureItemImpl(MotionCaptureItem *self, const MotionCaptureItemImpl& org)
    : self(self)
{
    isMotionDataRecordingEnabled = org.isMotionDataRecordingEnabled;
}


MotionCaptureItem::~MotionCaptureItem()
{
    delete impl;
}


void MotionCaptureItem::initializeClass(ExtensionManager* ext)
{
    ext->itemManager()
            .registerClass<MotionCaptureItem>(N_("MotionCaptureItem"))
            .addCreationPanel<MotionCaptureItem>();
}


bool MotionCaptureItem::initializeSimulation(SimulatorItem* simulatorItem)
{
    return impl->initializeSimulation(simulatorItem);
}


bool MotionCaptureItemImpl::initializeSimulation(SimulatorItem* simulatorItem)
{
    this->simulatorItem = simulatorItem;
    markers.clear();
    pointSetItems.clear();
    if(multiPointSetItem) {
        multiPointSetItem->setChecked(false);
    }

    const vector<SimulationBody*>& simBodies = simulatorItem->simulationBodies();
    for(size_t i = 0; i < simBodies.size(); ++i) {
        Body* body = simBodies[i]->body();
        markers << body->devices();
    }

    if(isMotionDataRecordingEnabled) {
        if(markers.size()) {
            multiPointSetItem = new MultiPointSetItem;
            multiPointSetItem->setName("Motion");
            multiPointSetItem->setChecked(false);
            self->addSubItem(multiPointSetItem);

            for(size_t i = 0; i < markers.size(); ++i) {
                PassiveMarker* marker = markers[i];
                PointSetItem* pointSetItem = new PointSetItem;
                pointSetItem->setName(marker->name());
                pointSetItem->setChecked(false);
                multiPointSetItem->addSubItem(pointSetItem);
                pointSetItems.push_back(pointSetItem);
            }

            motionSeqItem = new MultiSE3SeqItem;
            motionSeqItem->setName("MotionSeq");
            multiPointSetItem->addSubItem(motionSeqItem);

            int numParts = markers.size();
            shared_ptr<MultiSE3Seq> markerPointSeq = motionSeqItem->seq();
            markerPointSeq->setSeqContentName("MotionSeq");
            markerPointSeq->setFrameRate(1.0 / simulatorItem->worldTimeStep());
            markerPointSeq->setDimension(0, numParts, false);
            markerPointSeq->setOffsetTime(0.0);

            simulatorItem->addPreDynamicsFunction([&](){ onPreDynamicsFunction(); });
        }
    }
    return true;
}


void MotionCaptureItem::finalizeSimulation()
{
    impl->finalizeSimulation();
}


void MotionCaptureItemImpl::finalizeSimulation()
{
    if(multiPointSetItem && isMotionDataRecordingEnabled) {
        QDateTime recordingStartTime = QDateTime::currentDateTime();
        string suffix = recordingStartTime.toString("-yyyy-MM-dd-hh-mm-ss").toStdString();

        multiPointSetItem->setChecked(true);
        filesystem::path homeDir(fromUTF8(getenv("HOME")));
        string captureDirPath = toUTF8((homeDir / "capture" / (multiPointSetItem->name() + suffix).c_str()).string());
        filesystem::path dir(fromUTF8(captureDirPath));
        if(!filesystem::exists(dir)) {
            filesystem::create_directories(dir);
        }

        for(size_t i = 0; i < markers.size(); ++i) {
            PassiveMarker* marker = markers[i];
            vector<Vector3> src;
            PointSetItem* pointSetItem = pointSetItems[i];
            for(int i = 0; i < pointSetItem->numAttentionPoints(); ++i) {
                Vector3 point = pointSetItem->attentionPoint(i);
                src.push_back(point);
            }

            pointSetItem->clearAttentionPoints();
            SgVertexArray& points = *pointSetItem->pointSet()->getOrCreateVertices();
            SgColorArray& colors = *pointSetItem->pointSet()->getOrCreateColors();
            const int numPoints = src.size();
            points.resize(numPoints);
            colors.resize(numPoints);
            for(int i = 0; i < numPoints; ++i) {
                Vector3f point = Vector3f(src[i][0], src[i][1], src[i][2]);
                points[i] = point;
                Vector3f& c = colors[i];
                c[0] = marker->color()[0];
                c[1] = marker->color()[1];
                c[2] = marker->color()[2];
            }
            pointSetItem->notifyUpdate();
            string filename = toUTF8((dir / pointSetItem->name().c_str()).string()) + suffix + ".pcd";
            pointSetItem->save(filename);
        }

        string filename0 = toUTF8((dir / multiPointSetItem->name().c_str()).string()) + suffix + ".yaml";
        multiPointSetItem->save(filename0);
    }
}


void MotionCaptureItemImpl::onPreDynamicsFunction()
{
    int currentFrame = simulatorItem->currentFrame();
    shared_ptr<MultiSE3Seq> motionSeq = motionSeqItem->seq();
    motionSeq->setNumFrames(currentFrame);
    MultiSE3Seq::Frame p = motionSeq->frame(currentFrame - 1);

    for(size_t i = 0; i < markers.size(); ++i) {
        PassiveMarker* marker = markers[i];
        if(marker->on()) {
            Link* link = marker->link();
            Vector3 point = link->T() * marker->p_local();
            Matrix3 R = link->R() * marker->R_local();
            p[i].set(point, R);
            pointSetItems[i]->addAttentionPoint(point);
        }
    }
}


Item* MotionCaptureItem::doDuplicate() const
{
    return new MotionCaptureItem(*this);
}


void MotionCaptureItem::doPutProperties(PutPropertyFunction& putProperty)
{
    SubSimulatorItem::doPutProperties(putProperty);
    impl->doPutProperties(putProperty);
}


void MotionCaptureItemImpl::doPutProperties(PutPropertyFunction& putProperty)
{
    putProperty(_("Record motion data"), isMotionDataRecordingEnabled, changeProperty(isMotionDataRecordingEnabled));
}


bool MotionCaptureItem::store(Archive& archive)
{
    SubSimulatorItem::store(archive);
    return impl->store(archive);
}


bool MotionCaptureItemImpl::store(Archive& archive)
{
    return true;
}


bool MotionCaptureItem::restore(const Archive& archive)
{
    SubSimulatorItem::restore(archive);
    return impl->restore(archive);
}


bool MotionCaptureItemImpl::restore(const Archive& archive)
{
    return true;
}
