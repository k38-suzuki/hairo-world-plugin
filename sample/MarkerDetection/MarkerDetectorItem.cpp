/**
   @author Kenta Suzuki
*/

#include "MarkerDetectorItem.h"
#include <cnoid/Body>
#include <cnoid/DeviceList>
#include <cnoid/EigenTypes>
#include <cnoid/EigenUtil>
#include <cnoid/ItemManager>
#include <cnoid/PassiveMarker>
#include <cnoid/SimulatorItem>
#include "ScopeDevice.h"
#include "gettext.h"

using namespace cnoid;
using namespace std;

namespace cnoid {

class MarkerDetectorItem::Impl
{
public:
    MarkerDetectorItem* self;

    Impl(MarkerDetectorItem* self);
    Impl(MarkerDetectorItem* self, const Impl& org);

    DeviceList<PassiveMarker> markers;
    DeviceList<ScopeDevice> scopes;

    bool initializeSimulation(SimulatorItem* simulatorItem);
    void onPreDynamics();
    void doPutProperties(PutPropertyFunction& putProperty);
    bool store(Archive& archive);
    bool restore(const Archive& archive);
};

}


MarkerDetectorItem::MarkerDetectorItem()
{
    impl = new Impl(this);
}


MarkerDetectorItem::Impl::Impl(MarkerDetectorItem* self)
    : self(self)
{
    markers.clear();
    scopes.clear();
}


MarkerDetectorItem::MarkerDetectorItem(const MarkerDetectorItem& org)
    : SubSimulatorItem(org),
      impl(new Impl(this, *org.impl))

{

}


MarkerDetectorItem::Impl::Impl(MarkerDetectorItem* self, const Impl& org)
    : self(self)
{

}


MarkerDetectorItem::~MarkerDetectorItem()
{
    delete impl;
}


void MarkerDetectorItem::initializeClass(ExtensionManager* ext)
{
    ext->itemManager().registerClass<MarkerDetectorItem>(N_("MarkerDetectorItem"));
    ext->itemManager().addCreationPanel<MarkerDetectorItem>();
}


bool MarkerDetectorItem::initializeSimulation(SimulatorItem* simulatorItem)
{
    return impl->initializeSimulation(simulatorItem);
}


bool MarkerDetectorItem::Impl::initializeSimulation(SimulatorItem* simulatorItem)
{
    markers.clear();
    scopes.clear();

    const vector<SimulationBody*>& simBodies = simulatorItem->simulationBodies();
    for(size_t i = 0; i < simBodies.size(); ++i) {
        Body* body = simBodies[i]->body();
        markers << body->devices();
        scopes << body->devices();
    }

    if(markers.size() && scopes.size()) {
        simulatorItem->addPreDynamicsFunction([&](){ onPreDynamics(); });
    }
    return true;
}


void MarkerDetectorItem::Impl::onPreDynamics()
{
    DeviceList<PassiveMarker> capturedMarkers;
    for(size_t i = 0; i < scopes.size(); ++i) {
        ScopeDevice* scope = scopes[i];
        Link* clink = scope->link();
        double rangehy = scope->focalLength() * tan((double)scope->fieldOfView() / 2.0 * TO_RADIAN);
        double rangehz = rangehy * 2.0 / scope->aspectRatio()[0] * scope->aspectRatio()[1] / 2.0;
        Matrix3 m = clink->R() * scope->R_local();
        Vector3 cp = clink->T().translation() + m * scope->p_local();
        Vector3 o = cp + m * (Vector3(0.0, 0.0, 0.0));
        Vector3 a = cp + m * (Vector3(scope->focalLength(), rangehy,  rangehz));
        Vector3 b = cp + m * (Vector3(scope->focalLength(), rangehy, -rangehz));
        Vector3 c = cp + m * (Vector3(scope->focalLength(), -rangehy, -rangehz));
        Vector3 d = cp + m * (Vector3(scope->focalLength(), -rangehy,  rangehz));

        Vector3 oa = a - o;
        Vector3 ob = b - o;
        Vector3 oc = c - o;
        Vector3 od = d - o;

        for(size_t j = 0; j < markers.size(); ++j) {
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
    for(size_t i = 0; i < capturedMarkers.size(); ++i) {
        PassiveMarker* marker = capturedMarkers[i];
        marker->setTransparency(0.0);
        marker->notifyStateChange();
    }
}


Item* MarkerDetectorItem::doDuplicate() const
{
    return new MarkerDetectorItem(*this);
}


void MarkerDetectorItem::doPutProperties(PutPropertyFunction& putProperty)
{
    SubSimulatorItem::doPutProperties(putProperty);
    impl->doPutProperties(putProperty);
}


void MarkerDetectorItem::Impl::doPutProperties(PutPropertyFunction& putProperty)
{

}


bool MarkerDetectorItem::store(Archive& archive)
{
    SubSimulatorItem::store(archive);
    return impl->store(archive);
}


bool MarkerDetectorItem::Impl::store(Archive& archive)
{
    return true;
}


bool MarkerDetectorItem::restore(const Archive& archive)
{
    SubSimulatorItem::restore(archive);
    return impl->restore(archive);
}


bool MarkerDetectorItem::Impl::restore(const Archive& archive)
{
    return true;
}
