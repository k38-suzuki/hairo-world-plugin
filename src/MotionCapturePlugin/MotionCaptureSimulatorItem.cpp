/**
   \file
   \author Kenta Suzuki
*/

#include "MotionCaptureSimulatorItem.h"
#include <cnoid/Archive>
#include <cnoid/Body>
#include <cnoid/BodyItem>
#include <cnoid/EigenTypes>
#include <cnoid/EigenUtil>
#include <cnoid/Item>
#include <cnoid/ItemManager>
#include <cnoid/ItemTreeView>
#include <cnoid/ProjectManager>
#include <cnoid/PutPropertyFunction>
#include <cnoid/SimulatorItem>
#include <QDateTime>
#include <fstream>
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
    vector<PassiveMarker*> markers_;
    vector<MotionCaptureCamera*> cameras_;
    vector<Body*> bodies;
    MarkerPointItem* item;
    double cycleTime;
    bool exportCsv;
    bool record;
    ofstream ofs;
    double timeStep;
    string fileName;

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
    markers_.clear();
    cameras_.clear();

    item = nullptr;
    cycleTime = 0.1;
    exportCsv = false;
    record = true;
    timeStep = 0.0;
    fileName.clear();
}


MotionCaptureSimulatorItem::MotionCaptureSimulatorItem(const MotionCaptureSimulatorItem& org)
    : SubSimulatorItem(org),
      impl(new MotionCaptureSimulatorItemImpl(this, *org.impl))

{

}


MotionCaptureSimulatorItemImpl::MotionCaptureSimulatorItemImpl(MotionCaptureSimulatorItem* self, const MotionCaptureSimulatorItemImpl& org)
    : self(self)
{
    markers_ = org.markers_;
    cameras_ = org.cameras_;

    item = org.item;
    cycleTime = org.cycleTime;
    exportCsv = org.exportCsv;
    record = org.record;
    timeStep = org.timeStep;
    fileName = org.fileName;
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
    markers_.clear();
    cameras_.clear();
    bodies.clear();
    timeStep = simulatorItem->worldTimeStep();

    vector<SimulationBody*> simulationBodies = simulatorItem->simulationBodies();
    for(size_t i = 0; i < simulationBodies.size(); i++) {
        Body* body = simulationBodies[i]->body();
        bodies.push_back(body);

        DeviceList<PassiveMarker> markers = body->devices();
        for(size_t j = 0; j < markers.size(); j++) {
            PassiveMarker* marker = markers[j];
            markers_.push_back(marker);
        }
        DeviceList<MotionCaptureCamera> cameras = body->devices();
        for(size_t j = 0; j < cameras.size(); j++) {
            MotionCaptureCamera* camera = cameras[j];
            cameras_.push_back(camera);
        }
    }
    if(markers_.size() && !record) {
        simulatorItem->addPreDynamicsFunction([&](){ onMarkerDetection(); });
    }

    string header[5];
    header[0] = "Trajectories";
    header[1] = "100";
    header[2] = ",";
    header[3] = "Frame,SubFrame";
    header[4] = ",";

    if(bodies.size()) {
        for(size_t j = 0; j < bodies.size(); j++) {
            Body* body = bodies[j];
            DeviceList<PassiveMarker> markers(body->devices());
            if(markers.size()) {
                for(int k = 0; k < markers.size(); k++) {
                    PassiveMarker* marker = markers[k];
                    header[2] += "," + body->name() + ":" + marker->name() + ",,";
                    header[3] += ",X,Y,Z";
                    header[4] += ",mm,mm,mm";
                }
            }
        }

        QDateTime dateTime = QDateTime::currentDateTime();
        string date = dateTime.toString("yyyyMMdd_hhmmss").toStdString();

        fileName = ProjectManager::instance()->currentProjectDirectory() + "/" + date + ".csv";

        if(record) {
            item = new MarkerPointItem();
            item->setName(date);

            self->addChildItem(item);
            ItemTreeView::instance()->checkItem(item, false);
        }

        if(exportCsv) {
            ofs.open(fileName);
        }
        if(ofs) {
            for(int i = 0; i < 5; i++) {
                ofs << header[i] << endl;
            }
        }
        if(record) {
            simulatorItem->addPreDynamicsFunction([&](){ onMarkerGeneration(); });
        }
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

    if(ofs) {
        ofs.close();
    }
}


void MotionCaptureSimulatorItemImpl::onMarkerDetection()
{
    vector<PassiveMarker*> capturedMarkers;
    for(size_t i = 0; i < cameras_.size(); i++) {
        MotionCaptureCamera* camera = cameras_[i];
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

        for(size_t j = 0; j < markers_.size(); j++) {
            PassiveMarker* marker = markers_[j];
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
        string data;
        for(int j = 0; j < (int)bodies.size(); j++) {
            Body* body = bodies[j];
            DeviceList<PassiveMarker> markers(body->devices());
            for(int k = 0; k < markers.size(); k++) {
                PassiveMarker* marker = markers[k];
                if(marker->on()) {
                    Link* link = marker->link();
                    Vector3 point = (link->T() * marker->T_local()).translation();
//                        Vector3 point = link->T().translation() + link->R() * marker->R_local() * marker->p_local();
                    Vector3 color = marker->color();
                    if(record) {
                        item->addPoint(point, marker->radius(),
                                       Vector3f(color[0], color[1], color[2]), marker->transparency());
                    }
                    data += "," + to_string(point[0] * 1000.0) +
                            "," + to_string(point[1] * 1000.0) +
                            "," + to_string(point[2] * 1000.0);
                }
            }
        }

        if(ofs) {
            static int i = 0;
            ofs << i++ << ",0" << data << endl;
        }
        timeCounter -= cycleTime;
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
    if(record) {
        putProperty(_("CycleTime"), cycleTime, changeProperty(cycleTime));
        putProperty(_("Export CSV"), exportCsv, changeProperty(exportCsv));
    }
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
    archive.write("exportCsv", exportCsv);
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
    archive.read("exportCsv", exportCsv);
    return true;
}
