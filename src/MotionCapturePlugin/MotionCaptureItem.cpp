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
    vector<PointSetItem*> pointSetItems;
    MultiPointSetItemPtr multiPointSetItem;
    SimulatorItem* simulatorItem;
    MultiSE3SeqItemPtr motionSeqItem;
    bool isMotionDataRecordingEnabled;

    bool initializeSimulation(SimulatorItem* simulatorItem);
    void finalizeSimulation();
    void onPreDynamics();
};

}


MotionCaptureItem::MotionCaptureItem()
{
    impl = new Impl(this);
}


MotionCaptureItem::Impl::Impl(MotionCaptureItem* self)
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
      impl(new Impl(this, *org.impl))
{

}


MotionCaptureItem::Impl::Impl(MotionCaptureItem *self, const Impl& org)
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
            .registerClass<MotionCaptureItem, SubSimulatorItem>(N_("MotionCaptureItem"))
            .addCreationPanel<MotionCaptureItem>();
}


bool MotionCaptureItem::initializeSimulation(SimulatorItem* simulatorItem)
{
    return impl->initializeSimulation(simulatorItem);
}


bool MotionCaptureItem::Impl::initializeSimulation(SimulatorItem* simulatorItem)
{
    this->simulatorItem = simulatorItem;
    markers.clear();
    pointSetItems.clear();
    if(multiPointSetItem) {
        multiPointSetItem->setChecked(false);
    }

    const vector<SimulationBody*>& simBodies = simulatorItem->simulationBodies();
    for(auto& simBody : simBodies) {
        Body* body = simBody->body();
        markers << body->devices();
    }

    if(isMotionDataRecordingEnabled) {
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

            simulatorItem->addPreDynamicsFunction([&](){ onPreDynamics(); });
        }
    }
    return true;
}


void MotionCaptureItem::finalizeSimulation()
{
    impl->finalizeSimulation();
}


void MotionCaptureItem::Impl::finalizeSimulation()
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
            for(int j = 0; j < numPoints; ++j) {
                Vector3f point = Vector3f(src[i][0], src[i][1], src[i][2]);
                points[j] = point;
                Vector3f& c = colors[j];
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


void MotionCaptureItem::Impl::onPreDynamics()
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


Item* MotionCaptureItem::doCloneItem(CloneMap* cloneMap) const
{
    return new MotionCaptureItem(*this);
}


void MotionCaptureItem::doPutProperties(PutPropertyFunction& putProperty)
{
    SubSimulatorItem::doPutProperties(putProperty);
    putProperty(_("Record motion data"), impl->isMotionDataRecordingEnabled, changeProperty(impl->isMotionDataRecordingEnabled));
}


bool MotionCaptureItem::store(Archive& archive)
{
    SubSimulatorItem::store(archive);
    return true;
}


bool MotionCaptureItem::restore(const Archive& archive)
{
    SubSimulatorItem::restore(archive);
    return true;
}
