/**
   \file
   \author Kenta Suzuki
*/

#include "FluidDynamicsSimulatorItem.h"
#include <cnoid/Archive>
#include <cnoid/Body>
#include <cnoid/EigenArchive>
#include <cnoid/EigenUtil>
#include <cnoid/ItemManager>
#include <cnoid/ItemTreeView>
#include <cnoid/SimulatorItem>
#include <cnoid/WorldItem>
#include <vector>
#include <iostream>
#include <cmath>
#include "FDBody.h"
#include "FluidAreaItem.h"
#include "Rotor.h"
#include "Thruster.h"
#include "gettext.h"

using namespace std;
using namespace cnoid;

namespace cnoid {

class FluidDynamicsSimulatorItemImpl
{
public:
    FluidDynamicsSimulatorItemImpl(FluidDynamicsSimulatorItem* self);
    FluidDynamicsSimulatorItemImpl(FluidDynamicsSimulatorItem* self, const FluidDynamicsSimulatorItemImpl& org);

    FluidDynamicsSimulatorItem* self;
    FluidAreaItem* isCollided(const Link* link);
    Vector3 gravity;
    std::vector<FDBody*> fdBodies;
    ItemList<FluidAreaItem> items;
    bool initializeSimulation(SimulatorItem* simulatorItem);
    void doPutProperties(PutPropertyFunction& putProperty);
    bool store(Archive& archive);
    bool restore(const Archive& archive);
    void onPreDynamicsFunction();
    void createFDBody(Body* body);
};

}


FluidDynamicsSimulatorItem::FluidDynamicsSimulatorItem()
{
    impl = new FluidDynamicsSimulatorItemImpl(this);
}


FluidDynamicsSimulatorItemImpl::FluidDynamicsSimulatorItemImpl(FluidDynamicsSimulatorItem* self)
    : self(self)
{
    gravity << 0.0, 0.0, -9.80665;
    fdBodies.clear();
    items.clear();
}


FluidDynamicsSimulatorItem::FluidDynamicsSimulatorItem(const FluidDynamicsSimulatorItem& org)
    : SubSimulatorItem(org),
      impl(new FluidDynamicsSimulatorItemImpl(this, *org.impl))
{

}


FluidDynamicsSimulatorItemImpl::FluidDynamicsSimulatorItemImpl(FluidDynamicsSimulatorItem* self, const FluidDynamicsSimulatorItemImpl& org)
    : self(self)
{
    gravity = org.gravity;
    fdBodies = org.fdBodies;
    items = org.items;
}


FluidDynamicsSimulatorItem::~FluidDynamicsSimulatorItem()
{
    delete impl;
}


void FluidDynamicsSimulatorItem::initializeClass(ExtensionManager* ext)
{
    ext->itemManager().registerClass<FluidDynamicsSimulatorItem>(N_("FluidDynamicsSimulatorItem"));
    ext->itemManager().addCreationPanel<FluidDynamicsSimulatorItem>();
}


bool FluidDynamicsSimulatorItem::initializeSimulation(SimulatorItem* simulatorItem)
{
    return impl->initializeSimulation(simulatorItem);
}


bool FluidDynamicsSimulatorItemImpl::initializeSimulation(SimulatorItem* simulatorItem)
{
    gravity = simulatorItem->getGravity();
    fdBodies.clear();
    items.clear();
    vector<SimulationBody*> simulationBodies = simulatorItem->simulationBodies();
    for(size_t i = 0; i < simulationBodies.size(); i++) {
        createFDBody(simulationBodies[i]->body());
    }

    if(fdBodies.size()) {
//        cout << fdBodies.size() << endl;
        items = ItemTreeView::instance()->checkedItems<FluidAreaItem>();
        simulatorItem->addPreDynamicsFunction([&](){ onPreDynamicsFunction(); });
    }
    return true;
}


void FluidDynamicsSimulatorItemImpl::onPreDynamicsFunction()
{
    for(size_t i = 0; i < fdBodies.size(); i++) {
        FDBody* fdBody = fdBodies[i];
        Body* body = fdBodies[i]->body();
        for(int j = 0; j < fdBody->numFDLinks(); j++) {
            FDLink* fdLink = fdBody->fdLink(j);
            Link* link = fdLink->link();
            double density = 0.0;
            double viscosity = 0.0;
            Vector3 flow = Vector3::Zero();

            FluidAreaItem* item = isCollided(link);
            if(item) {
                density = item->density();
                viscosity = item->viscosity();
                flow = item->flow();
            }

            //flow
            Vector3 f_flow = flow;

            // buoyancy
            double volume = 0.0;
            if(fdLink->density() != 0.0) {
                volume = link->mass() / fdLink->density();
            }
            Vector3 f_buoyancy = density * gravity * volume * -1.0;
            Vector3 centerOfBuoyancy = link->R() * fdLink->centerOfBuoyancy();
            Vector3 tau_buoyancy = centerOfBuoyancy.cross(f_buoyancy);

            Vector3 v = link->v();
            Vector3 w = link->w();

            //drag
            Vector3 f_drag = Vector3::Zero();
            Vector3 tau_drag = Vector3::Zero();
            //viscous drag
            Vector3 f_visco = Vector3::Zero();
            Vector3 tau_visco = Vector3::Zero();
            double cd = 0.0;
            if(density > 10.0) {
                cd = fdLink->cdw();
            } else {
                cd = fdLink->cda();
            }
            for(int k = 0; k < 3; ++k) {
                double sign = 1.0;
                int index = 1;
                if(v[k] >= 0.0) {
                    sign = -1.0;
                    index = 0;
                }
                f_drag[k] = 0.5 * density * v[k] * v[k] * fdLink->surface()[k * 2 + index] * cd * sign;
                double cv = fdLink->cv();
                f_visco[k] = cv * viscosity * v[k] * sign;
            }
            for(int k = 0; k < 3; ++k) {
                double sign = 1.0;
                int index = 1;
                int index0 = (k + 1) % 3;
                int index1 = (k + 2) % 3;
                if(w[k] >= 0.0) {
                    sign = -1.0;
                    index = 0;
                }
                tau_drag[k] = density * w[k] * w[k] * (fdLink->surface()[index0 * 2 + index] + fdLink->surface()[index1 * 2 + index]) * fdLink->td() * sign;
                double cv = fdLink->cv();
                tau_visco[k] = cv * viscosity * w[k] * sign;
            }

            vector<Vector3> f_exts;
            f_exts.push_back(f_flow);
            f_exts.push_back(f_buoyancy);
            f_exts.push_back(f_drag);
            f_exts.push_back(f_visco);
            for(int k = 0; k < f_exts.size(); k++) {
                if(!isnan(f_exts[k].norm())) {
                    link->f_ext() += f_exts[k];
                }
            }

            vector<Vector3> tau_exts;
            tau_exts.push_back(tau_buoyancy);
            tau_exts.push_back(tau_drag);
            tau_exts.push_back(tau_visco);
            for(int k = 0; k < tau_exts.size(); k++) {
                if(!isnan(tau_exts[k].norm())) {
                    link->tau_ext() += tau_exts[k];
                }
            }
        }

        //thruster
        DeviceList<Thruster> thrusters = body->devices();
        for(int k = 0; k < thrusters.size(); k++) {
            Thruster* thruster = thrusters[k];
            Link* link = thruster->link();
            FluidAreaItem* item = isCollided(link);
            if(item) {
                double density = item->density();
                if(density > 10.0) {
                    const Matrix3 R = link->R() * thruster->R_local();
                    const Vector3 p = link->R() * thruster->p_local();
                    Quat rotation(R);
                    Vector3 f_ext = rotation * (Vector3::UnitX() * (thruster->force() + thruster->forceOffset()));
                    Vector3 tau_ext = rotation * (Vector3::UnitX() * (thruster->torque() + thruster->torqueOffset()) + p.cross(f_ext));
                    if(thruster->on()) {
                        link->f_ext() += f_ext;
                        link->tau_ext() += tau_ext;
                    }
                }
            }
        }

        //rotor
        DeviceList<Rotor> rotors = body->devices();
        for(int k = 0; k < rotors.size(); k++) {
            Rotor* rotor = rotors[k];
            Link* link = rotor->link();
            FluidAreaItem* item = isCollided(link);
            if(item) {
                double density = item->density();
                if(density < 10.0) {
                    const Matrix3 R = link->R() * rotor->R_local();
                    const Vector3 p = link->R() * rotor->p_local();
                    Quat rotation(R);
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
                    Vector3 f_ext = rotation * (Vector3::UnitZ() * (rotor->force() + rotor->forceOffset() + staticForce));
                    Vector3 tau_ext = rotation * (Vector3::UnitZ() * (rotor->torque() + rotor->torqueOffset()) + p.cross(f_ext));
                    if(rotor->on()) {
                        link->f_ext() += f_ext;
                        link->tau_ext() += tau_ext;
                    }
                }
            }
        }

        for(int k = 0; k < body->numLinks(); k++) {
            Link* link = body->link(k);
            link->tau_ext() += -1.0 * link->f_ext().cross(link->T().translation());
        }
    }
}


void FluidDynamicsSimulatorItemImpl::createFDBody(Body* body)
{
    FDBody* fdBody = new FDBody(body);
    for(int i = 0; i < body->numLinks(); i++) {
        Link* link = body->link(i);
        FDLink* fdLink = new FDLink(link);
        Mapping& node = *link->info();

        double d;
        Vector3 v;
        Vector6 v6;
        if(node.read("density", d)) fdLink->setDensity(d);
        if(read(node, "centerOfBuoyancy", v)) {
            fdLink->setCenterOfBuoyancy(v);
        } else {
            fdLink->setCenterOfBuoyancy(link->centerOfMass());
        }
        if(node.read("cdw", d)) fdLink->setCdw(d);
        if(node.read("cda", d)) fdLink->setCda(d);
        if(node.read("td", d)) fdLink->setTd(d);
        if(read(node, "surface", v6)) fdLink->setSurface(v6);
        if(node.read("cv", d)) fdLink->setCv(d);
        fdBody->addFDLinks(fdLink);
    }
    fdBodies.push_back(fdBody);
}


FluidAreaItem* FluidDynamicsSimulatorItemImpl::isCollided(const Link* link)
{
    FluidAreaItem* targetItem = nullptr;

    for(int k = 0; k < items.size(); k++) {
        FluidAreaItem* item = items[k];
        Vector3 p = link->T().translation();
        Vector3 translation = item->translation();
        Vector3 rpy = item->rotation() * TO_RADIAN;
        Matrix3 m = rotFromRpy(rpy);

        if(item->type() == "Box") {
            Vector3 size = item->size();
            Vector3 minRange = translation - size / 2.0;
            Vector3 maxRange = translation + size / 2.0;
            if((minRange[0] <= p[0]) && (p[0] <= maxRange[0])
                    && (minRange[1] <= p[1]) && (p[1] <= maxRange[1])
                    && (minRange[2] <= p[2]) && (p[2] <= maxRange[2])
                    ) {
                targetItem = item;
            }
        }
        else if(item->type() == "Cylinder") {
            Vector3 a = m * (Vector3(0.0, 1.0, 0.0) * item->height() / 2.0) + translation;
            Vector3 b = m * (Vector3(0.0, 1.0, 0.0) * item->height() / 2.0 * -1.0) + translation;
            Vector3 c = a - b;
            Vector3 d = p - b;
            double cd = c.dot(d);
            if((0.0 < cd) && (cd < c.dot(c))) {
                double r2 = d.dot(d) - d.dot(c) * d.dot(c) / c.dot(c);
                double rp2 =  item->radius() * item->radius();
                if(r2 < rp2) {
                    targetItem = item;
                }
            }
        }
        else if(item->type() == "Sphere") {
            Vector3 r = translation - p;
            if(r.norm() <= item->radius()) {
                targetItem = item;
            }
        }
        WorldItem* worldItem = item->findOwnerItem<WorldItem>();
        if(!worldItem) {
            targetItem = nullptr;
        }
    }

    return targetItem;
}


Item* FluidDynamicsSimulatorItem::doDuplicate() const
{
    return new FluidDynamicsSimulatorItem(*this);
}


void FluidDynamicsSimulatorItem::doPutProperties(PutPropertyFunction& putProperty)
{
    SubSimulatorItem::doPutProperties(putProperty);
    impl->doPutProperties(putProperty);
}

        
void FluidDynamicsSimulatorItemImpl::doPutProperties(PutPropertyFunction& putProperty)
{

}


bool FluidDynamicsSimulatorItem::store(Archive& archive)
{
    SubSimulatorItem::store(archive);
    return impl->store(archive);
}


bool FluidDynamicsSimulatorItemImpl::store(Archive& archive)
{
    return true;
}


bool FluidDynamicsSimulatorItem::restore(const Archive& archive)
{
    SubSimulatorItem::restore(archive);
    return impl->restore(archive);
}


bool FluidDynamicsSimulatorItemImpl::restore(const Archive& archive)
{
    return true;
}
