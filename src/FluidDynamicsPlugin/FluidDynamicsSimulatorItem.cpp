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
#include <cnoid/RootItem>
#include <cnoid/SimulatorItem>
#include <cnoid/WorldItem>
#include <vector>
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
    DeviceList<Thruster> thrusters;
    DeviceList<Rotor> rotors;

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
    thrusters.clear();
    rotors.clear();
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
    thrusters = org.thrusters;
    rotors = org.rotors;
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
    thrusters.clear();
    rotors.clear();
    vector<SimulationBody*> simulationBodies = simulatorItem->simulationBodies();
    for(size_t i = 0; i < simulationBodies.size(); i++) {
        Body* body = simulationBodies[i]->body();
        createFDBody(body);
        thrusters << body->devices();
        rotors << body->devices();
    }

    if(fdBodies.size()) {
        RootItem* rootItem = RootItem::instance();
        items = rootItem->checkedItems<FluidAreaItem>();
        simulatorItem->addPreDynamicsFunction([&](){ onPreDynamicsFunction(); });
    }
    return true;
}


void FluidDynamicsSimulatorItemImpl::onPreDynamicsFunction()
{
    for(size_t i = 0; i < fdBodies.size(); i++) {
        FDBody* fdBody = fdBodies[i];
        for(int j = 0; j < fdBody->numFDLinks(); j++) {
            FDLink* fdLink = fdBody->fdLink(j);
            Link* link = fdLink->link();
            double density = 0.0;
            double viscosity = 0.0;

            //flow
            Vector3 ff = Vector3::Zero();
            FluidAreaItem* item = isCollided(link);
            if(item) {
                density = item->density();
                viscosity = item->viscosity();
                ff = item->flow();
            }
            link->f_ext() += ff;
            Vector3 cr = link->T() * Vector3(0.0, 0.0, 0.0);
            link->tau_ext() += cr.cross(ff);

            // buoyancy
            double volume = 0.0;
            if(fdLink->density() != 0.0) {
                volume = link->mass() / fdLink->density();
            }
            Vector3 fb = density * gravity * volume * -1.0;
            link->f_ext() += fb;
            Vector3 cb = link->T() * fdLink->centerOfBuoyancy();
            link->tau_ext() += cb.cross(fb);

            Vector3 v = link->v();
            Vector3 w = link->w();
            double cd = 0.0;
            if(density > 10.0) {
                cd = fdLink->cdw();
            } else {
                cd = fdLink->cda();
            }

            //drag
            Vector3 fd = Vector3::Zero();
            Vector3 td = Vector3::Zero();
            //viscous drag
            Vector3 fv = Vector3::Zero();
            Vector3 tv = Vector3::Zero();

            // Force
            for(int k = 0; k < 3; ++k) {
                double sign = 1.0;
                int index = 1;
                if(v[k] >= 0.0) {
                    sign = -1.0;
                    index = 0;
                }
                fd[k] = 0.5 * density * v[k] * v[k] * fdLink->surface()[k * 2 + index] * cd * sign;
                double cv = fdLink->cv();
                fv[k] = cv * viscosity * fabs(v[k]) * sign;
            }

            // Moment
            for(int k = 0; k < 3; ++k) {
                double sign = 1.0;
                int index = 1;
                int index0 = (k + 1) % 3;
                int index1 = (k + 2) % 3;
                if(w[k] >= 0.0) {
                    sign = -1.0;
                    index = 0;
                }
                td[k] = density * w[k] * w[k] * (fdLink->surface()[index0 * 2 + index] + fdLink->surface()[index1 * 2 + index]) * fdLink->td() * sign;
                double cv = fdLink->cv();
                tv[k] = cv * viscosity * fabs(w[k]) * sign;
            }

            link->f_ext() += fd;
            link->tau_ext() += cr.cross(fd) + td;
            link->f_ext() += fv;
            link->tau_ext() += cr.cross(fv) + tv;
        }
    }

    // Thruster
    for(int k = 0; k < thrusters.size(); k++) {
        Thruster* thruster = thrusters[k];
        Link* link = thruster->link();
        FluidAreaItem* item = isCollided(link);
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

    // Rotor
    for(int k = 0; k < rotors.size(); k++) {
        Rotor* rotor = rotors[k];
        Link* link = rotor->link();
        FluidAreaItem* item = isCollided(link);
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
                if(rotor->on()) {
                    link->f_ext() += f;
                    link->tau_ext() += p.cross(f) + tau_ext;
                }
            }
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

        if(item->type() == AreaItem::BOX) {
            Vector3 size = item->size();
            Vector3 minRange = translation - size / 2.0;
            Vector3 maxRange = translation + size / 2.0;
            if((minRange[0] <= p[0]) && (p[0] <= maxRange[0])
                    && (minRange[1] <= p[1]) && (p[1] <= maxRange[1])
                    && (minRange[2] <= p[2]) && (p[2] <= maxRange[2])
                    ) {
                targetItem = item;
            }
        } else if(item->type() == AreaItem::CYLINDER) {
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
        } else if(item->type() == AreaItem::SPHERE) {
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
