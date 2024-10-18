/**
   @author Kenta Suzuki
*/

#include "CFDSimulatorItem.h"
#include <cnoid/Archive>
#include <cnoid/Body>
#include <cnoid/DeviceList>
#include <cnoid/EigenArchive>
#include <cnoid/ItemManager>
#include <cnoid/MathUtil>
#include <cnoid/MeshExtractor>
#include <cnoid/PutPropertyFunction>
#include <cnoid/SceneDrawables>
#include <cnoid/SimulatorItem>
#include <cnoid/WorldItem>
#include <cnoid/MultiColliderItem>
#include <vector>
#include "FlightEventReader.h"
#include "Rotor.h"
#include "Thruster.h"
#include "gettext.h"

using namespace std;
using namespace cnoid;

namespace {

const double DEFAULT_GRAVITY_ACCELERATION = 9.80665;

class CFDBody;

class CFDLink : public Referenced
{
public:
    CFDLink(CFDSimulatorItemImpl* simImpl, CFDBody* cfdBody, Link* link);
    ~CFDLink();

    Link* link;
    double density;
    Vector3 centerOfBuoyancy;
    double cdw;
    double cda;
    double cv;
    double cw;
    vector<Vector3> sn;
    vector<Vector3> g;

    void calcGeometry(CFDBody* cfdBody);
    void calcMesh(MeshExtractor* extractor, CFDBody* cfdBody);
};

typedef ref_ptr<CFDLink> CFDLinkPtr;

class CFDBody : public SimulationBody
{
public:
    CFDBody(Body* body);
    ~CFDBody();

    CFDLink* cfdLink(const int& index) { return cfdLinks[index]; }
    size_t numCFDLinks() const { return cfdLinks.size(); }

    vector<CFDLinkPtr> cfdLinks;

    void createBody(CFDSimulatorItemImpl* simImpl);
    void updateDevices();
};

typedef ref_ptr<CFDBody> CFDBodyPtr;

struct BatteryInfo {
    Rotor* rotor;
    double duration;
};

}

namespace cnoid {

class CFDSimulatorItemImpl
{
public:
    CFDSimulatorItem* self;

    CFDSimulatorItemImpl(CFDSimulatorItem* self);
    CFDSimulatorItemImpl(CFDSimulatorItem* self, const CFDSimulatorItemImpl& org);

    vector<CFDBody*> cfdBodies;
    DeviceList<Thruster> thrusters;
    DeviceList<Rotor> rotors;
    Vector3 gravity;
    ItemList<MultiColliderItem> colliders;
    vector<BatteryInfo> batteryInfo;
    string flight_event_file_path;
    vector<FlightEvent> events;

    double world_time_step;

    bool initializeSimulation(SimulatorItem* simulatorItem);
    void addBody(CFDBody* cfdBody);
    void onPreDynamics();
};

}


CFDLink::CFDLink(CFDSimulatorItemImpl* simImpl, CFDBody* cfdBody, Link* link)
{
    this->link = link;
    density = 0.0;
    centerOfBuoyancy << 0.0, 0.0, 0.0;
    cdw = 0.0;
    cda = 0.0;
    cv = 0.0;
    cw = 0.0;
    sn.clear();
    g.clear();
}


CFDLink::~CFDLink()
{

}


void CFDLink::calcGeometry(CFDBody* cfdBody)
{
    if(link->collisionShape()) {
        MeshExtractor* extractor = new MeshExtractor;

        if(extractor->extract(link->collisionShape(),
            [this, extractor, cfdBody](){ calcMesh(extractor, cfdBody); })) {

        }
        delete extractor;
    }
}


void CFDLink::calcMesh(MeshExtractor* extractor, CFDBody* cfdBody)
{
    SgMesh* mesh = extractor->currentMesh();
    const Affine3& T = extractor->currentTransform();

    const SgVertexArray& vertices_ = *mesh->vertices();
    const int numVertices = vertices_.size();
    for(int i = 0; i < numVertices; ++i) {
        const Vector3 v = vertices_[i].cast<Isometry3::Scalar>();
    }

    const int numTriangles = mesh->numTriangles();
    for(int i = 0; i < numTriangles; ++i) {
        SgMesh::TriangleRef src = mesh->triangle(i);
        Vector3 a = vertices_[src[0]].cast<Isometry3::Scalar>();
        Vector3 b = vertices_[src[1]].cast<Isometry3::Scalar>();
        Vector3 c = vertices_[src[2]].cast<Isometry3::Scalar>();
        const Vector3 v0 = a - c;
        const Vector3 v1 = b - c;
        double s = 0.5 * sqrt(v0.norm() * v0.norm() * v1.norm() * v1.norm() - v0.dot(v1) * v0.dot(v1));
        Vector3 n = v0.cross(v1).normalized();
        sn.push_back(n * s);
        g.push_back((a + b + c) / 3.0);
    }
}


CFDBody::CFDBody(Body* body)
    : SimulationBody(body)
{
    cfdLinks.clear();
}


CFDBody::~CFDBody()
{

}


void CFDBody::createBody(CFDSimulatorItemImpl* simImpl)
{
    Body* body = this->body();

    for(int i = 0; i < body->numLinks(); ++i) {
        Link* link = body->link(i);
        CFDLink* cfdLink = new CFDLink(simImpl, this, link);

        Mapping& node = *link->info();
        node.read("density", cfdLink->density);
        if(!read(node, { "center_of_buoyancy", "centerOfBuoyancy" }, cfdLink->centerOfBuoyancy)) {
            cfdLink->centerOfBuoyancy = link->centerOfMass();
        }
        node.read("cdw", cfdLink->cdw);
        node.read("cda", cfdLink->cda);
        node.read("cv", cfdLink->cv);
        node.read("cw", cfdLink->cw);

        cfdLink->calcGeometry(this);
        cfdLinks.push_back(cfdLink);
        // simImpl->cfdBodies.push_back(this);
    }
}


void CFDBody::updateDevices()
{
    const DeviceList<Thruster>& thrusters = this->body()->devices();
    for(auto& thruster : thrusters) {
        const Link* link = thruster->link();
    }

    const DeviceList<Rotor>& rotors = this->body()->devices();
    for(auto& rotor : rotors) {
        const Link* link = rotor->link();
    }
}


void CFDSimulatorItem::initializeClass(ExtensionManager* ext)
{
    ext->itemManager()
        .registerClass<CFDSimulatorItem, SubSimulatorItem>(N_("CFDSimulatorItem"))
        .addCreationPanel<CFDSimulatorItem>();
}


CFDSimulatorItem::CFDSimulatorItem()
    : SubSimulatorItem()
{
    impl = new CFDSimulatorItemImpl(this);
}


CFDSimulatorItemImpl::CFDSimulatorItemImpl(CFDSimulatorItem* self)
    : self(self),
      world_time_step(0.0),
      flight_event_file_path("")
{
    cfdBodies.clear();
    thrusters.clear();
    rotors.clear();
    colliders.clear();
    batteryInfo.clear();
    events.clear();

    gravity << 0.0, 0.0, -DEFAULT_GRAVITY_ACCELERATION;
}


CFDSimulatorItem::CFDSimulatorItem(const CFDSimulatorItem& org)
    : SubSimulatorItem(org),
      impl(new CFDSimulatorItemImpl(this, *org.impl))
{

}


CFDSimulatorItemImpl::CFDSimulatorItemImpl(CFDSimulatorItem* self, const CFDSimulatorItemImpl& org)
    : self(self)
{
    gravity = org.gravity;
    flight_event_file_path = org.flight_event_file_path;
}


CFDSimulatorItem::~CFDSimulatorItem()
{
    delete impl;
}


bool CFDSimulatorItem::initializeSimulation(SimulatorItem* simulatorItem)
{
    return impl->initializeSimulation(simulatorItem);
}


bool CFDSimulatorItemImpl::initializeSimulation(SimulatorItem* simulatorItem)
{
    cfdBodies.clear();
    thrusters.clear();
    rotors.clear();
    colliders.clear();
    batteryInfo.clear();
    events.clear();
    gravity = simulatorItem->getGravity();
    world_time_step = simulatorItem->worldTimeStep();

    if(!flight_event_file_path.empty()) {
        FlightEventReader reader;
        if(reader.load(flight_event_file_path)) {
            events = reader.events();
        }
    }

    const vector<SimulationBody*>& simBodies = simulatorItem->simulationBodies();
    for(auto& simBody : simBodies) {
        Body* body = simBody->body();
        // addBody(static_cast<CFDBody*>(simBodies[i]));
        CFDBody* cfdBody = new CFDBody(body);
        cfdBody->createBody(this);
        cfdBodies.push_back(cfdBody);
        thrusters << body->devices();
        rotors << body->devices();
    }

    for(auto& rotor : rotors) {
        Link* link = rotor->link();
        Body* body = link->body();
        double mass = body->mass();
        double duration = 0.0;

        for(auto& event : events) {
            if(mass < event.mass()) {
                duration = event.duration();
            }
        }
        if(duration > 0.0) {
            batteryInfo.push_back({ rotor, duration });
        }
    }

    WorldItem* worldItem = simulatorItem->findOwnerItem<WorldItem>();
    if(worldItem) {
        ItemList<MultiColliderItem> list = worldItem->descendantItems<MultiColliderItem>();
        for(auto& collider : list) {
            if(collider->colliderType() == MultiColliderItem::CFD) {
                colliders.push_back(collider);
            }
        }
    }
    for(auto& collider : colliders) {
        collider->setUnsteadyFlow(Vector3(0.0, 0.0, 0.0));
    }

    if(cfdBodies.size()) {
        simulatorItem->addPreDynamicsFunction([&](){ onPreDynamics(); });
    }
    return true;
}


void CFDSimulatorItemImpl::addBody(CFDBody* cfdBody)
{
    Body& body = *cfdBody->body();

    body.clearExternalForces();
    cfdBody->createBody(this);
}


void CFDSimulatorItemImpl::onPreDynamics()
{
    for(auto& cfdBody : cfdBodies) {
        for(int j = 0; j < cfdBody->numCFDLinks(); ++j) {
            CFDLink* cfdLink = cfdBody->cfdLink(j);
            Link* link = cfdLink->link;
            const Isometry3& T = link->T();

            double density = 0.0;
            double viscosity = 0.0;
            Vector3 sf = Vector3::Zero();
            for(auto& collider : colliders) {
                auto rot = collider->position().linear();
                if(collision(collider, T.translation())) {
                    density = collider->density();
                    viscosity = collider->viscosity();
                    sf += rot * collider->steadyFlow();
                    sf += rot * collider->unsteadyFlow();
                }
            }

            // buoyancy
            if(cfdLink->density > 0.0) {
                double volume = link->mass() / cfdLink->density;
                Vector3 b = density * gravity * volume * -1.0;
                link->f_ext() += b;
                Vector3 cb = T * cfdLink->centerOfBuoyancy;
                link->tau_ext() += cb.cross(b);
            }

            //flow
            link->f_ext() += sf;
            Vector3 c = T * link->centerOfMass();
            link->tau_ext() += c.cross(sf);

            //drag
            double cd = 0.0;
            if(density > 10.0) {
                cd = cfdLink->cdw;
            } else {
                cd = cfdLink->cda;
            }

            Vector3 a = link->R() * link->centerOfMass();
            Vector3 w = link->w();
            Vector3 v = link->v() + w.cross(a);

            double v_norm = v.dot(v);
            double v2 = v_norm * v_norm;
            // Vector3 v_local = link->R().inverse() * v;
            Vector3 n = v.normalized();
            double p = 0.5 * density * v2;

            for(int k = 0; k < cfdLink->sn.size(); ++k) {
                Vector3 sn = link->R() * cfdLink->sn[k];
                double s = n.dot(sn);
                if(s > 0.0) {
                    Vector3 f = p * cd * s * n * -1.0;
                    link->f_ext() += f;
                    link->tau_ext() += c.cross(f);
                    Vector3 g = T * cfdLink->g[k];
                    // link->tau_ext() += g.cross(f);
                }
            }

            //viscous drag
            Vector3 fv = cfdLink->cv * viscosity * v * -1.0;
            Vector3 tv = cfdLink->cw * viscosity * w * -1.0;
            link->f_ext() += fv;
            link->tau_ext() += c.cross(fv) + tv;
        }

        cfdBody->updateDevices();
    }

    // thruster
    for(auto& thruster : thrusters) {
        Link* link = thruster->link();
        MultiColliderItem* item = nullptr;
        for(auto& collider : colliders) {
            if(collision(collider, link->T().translation())) {
                item = collider;
            }
        }

        if(item) {
            double density = item->density();
            if(density > 10.0) {
                Matrix3 R = link->R() * thruster->R_local();
                Vector3 direction = thruster->direction();
                const Vector3 f = R * (direction * (thruster->force() + thruster->forceOffset()));
                const Vector3 p = link->T() * thruster->p_local();
                Vector3 tau_ext = R * (direction * (thruster->torque() + thruster->torqueOffset()));
                if(thruster->on()) {
                    link->f_ext() += f;
                    link->tau_ext() += p.cross(f) + tau_ext;
                }
            }
        }
    }

    // rotor
    for(auto& rotor : rotors) {
        Link* link = rotor->link();
        MultiColliderItem* item = nullptr;
        for(auto& collider : colliders) {
            if(collision(collider, link->T().translation())) {
                item = collider;
            }
        }

        bool is_battery_empty = false;
        if(events.size()) {
            for(auto& info : batteryInfo) {
                if(info.rotor == rotor) {
                    info.duration -= world_time_step;
                    if(info.duration < 0.0) {
                        is_battery_empty = true;
                    }
                }
            }
        }

        if(item) {
            double density = item->density();
            if(density < 10.0) {
                double n = rotor->kv() * rotor->voltage();
                double d3 = rotor->diameter() / 10.0;
                double p1 = rotor->pitch() / 10.0;
                double k = rotor->k();
                double g = fabs(gravity[2]);
                if(n <= 0.0) {
                    n = link->dq_target() * 30.0 / PI;
                } else {
                    double dir = 1.0;
                    if(rotor->reverse()) {
                        dir *= -1.0;
                    }
                    link->dq_target() = n * PI / 30.0 * dir;
                }
                double n2 = n / 1000.0;
                double staticForce = k * d3 * d3 * d3 * p1 * n2 * n2 * g / 1000.0;

                Matrix3 R = link->R() * rotor->R_local();
                Vector3 direction = rotor->direction();
                const Vector3 f = R * (direction * (rotor->force() + rotor->forceOffset() + staticForce));
                const Vector3 p = link->T() * rotor->p_local();
                Vector3 tau_ext = R * (direction * (rotor->torque() + rotor->torqueOffset()));
                if(rotor->on() && !is_battery_empty) {
                    link->f_ext() += f;
                    link->tau_ext() += p.cross(f) + tau_ext;
                }
            }
        }
    }
}


Item* CFDSimulatorItem::doCloneItem(CloneMap* cloneMap) const
{
    return new CFDSimulatorItem(*this);
}


void CFDSimulatorItem::doPutProperties(PutPropertyFunction& putProperty)
{
    SubSimulatorItem::doPutProperties(putProperty);
    putProperty(_("Flight event file"), FilePathProperty(impl->flight_event_file_path),
                [this](const std::string& value){
                    impl->flight_event_file_path = value;
                    return true;
                });
}


bool CFDSimulatorItem::store(Archive& archive)
{
    if(!SubSimulatorItem::store(archive)) {
        return false;
    }
    archive.writeRelocatablePath("flight_event_file_path", impl->flight_event_file_path);
    return true;
}


bool CFDSimulatorItem::restore(const Archive& archive)
{
    if(!SubSimulatorItem::restore(archive)) {
        return false;
    }
    string symbol;
    if(archive.read("flight_event_file_path", symbol)) {
        symbol = archive.resolveRelocatablePath(symbol);
        if(!symbol.empty()) {
            impl->flight_event_file_path = symbol;
        }
    }
    return true;
}
