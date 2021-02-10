/**
   \file
   \author Kenta Suzuki
*/

#include "MotionCaptureSimulatorItem.h"
#include <cnoid/Archive>
#include <cnoid/EigenTypes>
#include <cnoid/EigenUtil>
#include <cnoid/Item>
#include <cnoid/ItemManager>
#include <cnoid/ItemTreeView>
#include <cnoid/PutPropertyFunction>
#include <cnoid/SimulatorItem>
#include <QDateTime>
#include "gettext.h"
#include "MarkerPointItem.h"
#include "MotionCaptureCamera.h"
#include "PassiveMarker.h"

using namespace std;
using namespace cnoid;

namespace cnoid {

class MotionCaptureSimulatorItemImpl
{
public:
    MotionCaptureSimulatorItemImpl(MotionCaptureSimulatorItem* self);
    MotionCaptureSimulatorItemImpl(MotionCaptureSimulatorItem* self, const MotionCaptureSimulatorItemImpl& org);

    MotionCaptureSimulatorItem* self;
    DeviceList<PassiveMarker> markers;
    DeviceList<MotionCaptureCamera> cameras;
    MarkerPointItem* item;
    double cycleTime;
    bool record;
    double timeStep;
    string fileName;
    int frame;

    bool initializeSimulation(SimulatorItem* simulatorItem);
    void finalizeSimulation();
    void doPutProperties(PutPropertyFunction& putProperty);
    bool store(Archive& archive);
    bool restore(const Archive& archive);
    void onMarkerDetection();
    void onMarkerGeneration();
};

}


MotionCaptureSimulatorItem::MotionCaptureSimulatorItem()
{
    impl = new MotionCaptureSimulatorItemImpl(this);
}


MotionCaptureSimulatorItemImpl::MotionCaptureSimulatorItemImpl(MotionCaptureSimulatorItem* self)
    : self(self)
{
    markers.clear();
    cameras.clear();

    item = nullptr;
    cycleTime = 0.1;
    record = true;
    timeStep = 0.0;
    fileName.clear();
    frame = 0;
}


MotionCaptureSimulatorItem::MotionCaptureSimulatorItem(const MotionCaptureSimulatorItem& org)
    : SubSimulatorItem(org),
      impl(new MotionCaptureSimulatorItemImpl(this, *org.impl))

{

}


MotionCaptureSimulatorItemImpl::MotionCaptureSimulatorItemImpl(MotionCaptureSimulatorItem* self, const MotionCaptureSimulatorItemImpl& org)
    : self(self)
{
    markers = org.markers;
    cameras = org.cameras;

    item = org.item;
    cycleTime = org.cycleTime;
    record = org.record;
    timeStep = org.timeStep;
    fileName = org.fileName;
    frame = org.frame;
}


MotionCaptureSimulatorItem::~MotionCaptureSimulatorItem()
{
    delete impl;
}


void MotionCaptureSimulatorItem::initializeClass(ExtensionManager* ext)
{
    ext->itemManager().registerClass<MotionCaptureSimulatorItem>(N_("MotionCaptureSimulatorItem"));
    ext->itemManager().addCreationPanel<MotionCaptureSimulatorItem>();
}


bool MotionCaptureSimulatorItem::initializeSimulation(SimulatorItem* simulatorItem)
{
    return impl->initializeSimulation(simulatorItem);
}


bool MotionCaptureSimulatorItemImpl::initializeSimulation(SimulatorItem* simulatorItem)
{
    markers.clear();
    cameras.clear();
    timeStep = simulatorItem->worldTimeStep();
    frame = 0;

    vector<SimulationBody*> simulationBodies = simulatorItem->simulationBodies();
    for(size_t i = 0; i < simulationBodies.size(); i++) {
        Body* body = simulationBodies[i]->body();
        markers << body->devices();
        cameras << body->devices();
    }

    QDateTime dateTime = QDateTime::currentDateTime();
    string date = dateTime.toString("yyyyMMdd_hhmmss").toStdString();

    if(record) {
        item = new MarkerPointItem();
        item->setName(date);
        self->addChildItem(item);
        ItemTreeView::instance()->checkItem(item, false);
        int numParts = markers.size();
        shared_ptr<MultiSE3Seq> markerPosSeq = item->seq();
        markerPosSeq->setSeqContentName("MarkerPosSeq");
        markerPosSeq->setNumParts(numParts);
        markerPosSeq->setDimension(0, numParts, 1);
        markerPosSeq->setFrameRate(1.0 / cycleTime);

        for(size_t i = 0; i < markers.size(); ++i) {
            PassiveMarker* marker = markers[i];
            Body* body = marker->body();
            string label = body->name() + ":" + marker->name();
            item->addLabel(label);
        }

        simulatorItem->addPreDynamicsFunction([&](){ onMarkerGeneration(); });
    } else {
        simulatorItem->addPreDynamicsFunction([&](){ onMarkerDetection(); });
    }
    return true;
}


void MotionCaptureSimulatorItem::finalizeSimulation()
{
    impl->finalizeSimulation();
}


void MotionCaptureSimulatorItemImpl::finalizeSimulation()
{
    ItemTreeView::instance()->checkItem(item, true);
}


void MotionCaptureSimulatorItemImpl::onMarkerDetection()
{
    vector<PassiveMarker*> capturedMarkers;
    for(size_t i = 0; i < cameras.size(); i++) {
        MotionCaptureCamera* camera = cameras[i];
        Link* clink = camera->link();
        double rangehy = camera->focalLength() * tan((double)camera->fieldOfView() / 2.0 * TO_RADIAN);
        double rangehz = rangehy * 2.0 / camera->aspectRatio()[0] * camera->aspectRatio()[1] / 2.0;
        Matrix3 m = clink->R() * camera->R_local();
        Vector3 cp = clink->T().translation() + m * camera->p_local();
        Vector3 o = cp + m * (Vector3(0.0, 0.0, 0.0));
        Vector3 a = cp + m * (Vector3(camera->focalLength(), rangehy,  rangehz));
        Vector3 b = cp + m * (Vector3(camera->focalLength(), rangehy, -rangehz));
        Vector3 c = cp + m * (Vector3(camera->focalLength(), -rangehy, -rangehz));
        Vector3 d = cp + m * (Vector3(camera->focalLength(), -rangehy,  rangehz));

        Vector3 oa = a - o;
        Vector3 ob = b - o;
        Vector3 oc = c - o;
        Vector3 od = d - o;

        for(size_t j = 0; j < markers.size(); j++) {
            PassiveMarker* marker = markers[j];
            Link* mlink = marker->link();
            Vector3 mp = mlink->T().translation() + mlink->R() * marker->R_local() * marker->p_local();
            Vector3 op = mp - o;
            Vector3 ap = mp - a;
            Vector3 bp = mp - b;
            Vector3 cp = mp - c;
            Vector3 dp = mp - d;

            double voabc = fabs(oa.cross(ob).dot(oc)) / 6.0;
            double vpoab = fabs(-op.cross(-ap).dot(-bp)) / 6.0;
            double vpobc = fabs(-op.cross(-bp).dot(-cp)) / 6.0;
            double vpoca = fabs(-op.cross(-cp).dot(-ap)) / 6.0;
            double vpabc = fabs(-ap.cross(-bp).dot(-cp)) / 6.0;

            double voacd = fabs(oa.cross(oc).dot(od)) / 6.0;
            double vpoac = fabs(-op.cross(-ap).dot(-cp)) / 6.0;
            double vpocd = fabs(-op.cross(-cp).dot(-dp)) / 6.0;
            double vpoda = fabs(-op.cross(-dp).dot(-ap)) / 6.0;
            double vpacd = fabs(-ap.cross(-cp).dot(-dp)) / 6.0;

            int v0 = voabc * 1000.0;
            int v1 = (vpoab + vpobc + vpoca + vpabc) * 1000.0;
            int v2 = voacd * 1000.0;
            int v3 = (vpoac + vpocd + vpoda + vpacd) * 1000.0;

            if((v0 >= v1) || (v2 >= v3)) {
                capturedMarkers.push_back(marker);
            }
            marker->setTransparency(0.9);
            marker->notifyStateChange();
        }
    }
    for(size_t i = 0; i < capturedMarkers.size(); i++) {
        PassiveMarker* marker = capturedMarkers[i];
        marker->setTransparency(0.0);
        marker->notifyStateChange();
    }
}


void MotionCaptureSimulatorItemImpl::onMarkerGeneration()
{
    static double timeCounter = 0.0;
    timeCounter += timeStep;

    if(timeCounter >= cycleTime) {
        shared_ptr<MultiSE3Seq> markerPosSeq = item->seq();
        markerPosSeq->setNumFrames(frame + 1);
        MultiSE3Seq::Frame p = markerPosSeq->frame(frame);

        for(size_t i = 0; i < markers.size(); ++i) {
            PassiveMarker* marker = markers[i];
            if(marker->on()) {
                Link* link = marker->link();
                Vector3 point = link->T() * marker->p_local();
                Matrix3 R = link->R() * marker->R_local();
                Vector3 color = marker->color();
                p[i].set(point, R);
                item->addPoint(point, marker->radius(),
                               Vector3f(color[0], color[1], color[2]), marker->transparency());
            }
        }

        timeCounter -= cycleTime;
        frame++;
    }
}


Item* MotionCaptureSimulatorItem::doDuplicate() const
{
    return new MotionCaptureSimulatorItem(*this);
}


void MotionCaptureSimulatorItem::doPutProperties(PutPropertyFunction& putProperty)
{
    SubSimulatorItem::doPutProperties(putProperty);
    impl->doPutProperties(putProperty);
}


void MotionCaptureSimulatorItemImpl::doPutProperties(PutPropertyFunction& putProperty)
{
    putProperty(_("Record"), record, changeProperty(record));
    putProperty(_("CycleTime"), cycleTime, changeProperty(cycleTime));
}


bool MotionCaptureSimulatorItem::store(Archive& archive)
{
    SubSimulatorItem::store(archive);
    return impl->store(archive);
}


bool MotionCaptureSimulatorItemImpl::store(Archive& archive)
{
    archive.write("record", record);
    archive.write("cycleTime", cycleTime);
    return true;
}


bool MotionCaptureSimulatorItem::restore(const Archive& archive)
{
    SubSimulatorItem::restore(archive);
    return impl->restore(archive);
}


bool MotionCaptureSimulatorItemImpl::restore(const Archive& archive)
{
    archive.read("record", record);
    archive.read("cycleTime", cycleTime);
    return true;
}
