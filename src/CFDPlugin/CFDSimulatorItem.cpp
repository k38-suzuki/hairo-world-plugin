/**
   \file
   \author Kenta Suzuki
*/

#include "CFDSimulatorItem.h"
#include <cnoid/Archive>
#include <cnoid/Body>
#include <cnoid/EigenArchive>
#include <cnoid/EigenUtil>
#include <cnoid/ItemManager>
#include <cnoid/SimulatorItem>
#include <cnoid/WorldItem>
#include <vector>
#include <cmath>
#include "FluidAreaItem.h"
#include "Rotor.h"
#include "Thruster.h"
#include "gettext.h"

using namespace cnoid;
using namespace std;

namespace {

class CFDLink : public Referenced
{
public:
    CFDLink();
    virtual ~CFDLink();

    Link* link;
    double density;
    Vector3 centerOfBuoyancy;
    double cdw;
    double cda;
    double td;
    Vector6 surface;
    double cv;
};

typedef ref_ptr<CFDLink> CFDLinkPtr;


class CFDBody : public SimulationBody
{
public:
    CFDBody(Body* body);
    virtual ~CFDBody();

    CFDLink* createLink();
    CFDLink* link(const int& index) { return cfdLinks[index]; }
    size_t numLinks() const { return cfdLinks.size(); }

    std::vector<CFDLinkPtr> cfdLinks;
};

typedef ref_ptr<CFDBody> CFDBodyPtr;

}


namespace cnoid {

class CFDSimulatorItemImpl
{
public:
    CFDSimulatorItemImpl(CFDSimulatorItem* self);
    CFDSimulatorItemImpl(CFDSimulatorItem* self, const CFDSimulatorItemImpl& org);
    CFDSimulatorItem* self;

    vector<CFDBody*> cfdBodies;
    DeviceList<Thruster> thrusters;
    DeviceList<Rotor> rotors;
    SimulatorItem* simulatorItem;

    bool initializeSimulation(SimulatorItem* simulatorItem);
    void createCFDBody(Body* body);
    void onPreDynamicsFunction();
    void doPutProperties(PutPropertyFunction& putProperty);
    bool store(Archive& archive);
    bool restore(const Archive& archive);
};

}


CFDLink::CFDLink()
{
    link = nullptr;
    density = 0.0;
    centerOfBuoyancy << 0.0, 0.0, 0.0;
    cdw = 0.0;
    cda = 0.0;
    td = 0.0;
    surface << 0.0, 0.0, 0.0, 0.0, 0.0, 0.0;
    cv = 0.0;
}


CFDLink::~CFDLink()
{

}


CFDBody::CFDBody(Body* body)
    : SimulationBody(body)
{
    cfdLinks.clear();
}


CFDBody::~CFDBody()
{

}


CFDLink* CFDBody::createLink()
{
    CFDLink* cfdLink = new CFDLink;
    cfdLinks.push_back(cfdLink);
    return cfdLink;
}


CFDSimulatorItem::CFDSimulatorItem()
{
    impl = new CFDSimulatorItemImpl(this);
}


CFDSimulatorItemImpl::CFDSimulatorItemImpl(CFDSimulatorItem* self)
    : self(self)
{
    cfdBodies.clear();
    thrusters.clear();
    rotors.clear();
    simulatorItem = nullptr;
}


CFDSimulatorItem::CFDSimulatorItem(const CFDSimulatorItem& org)
    : SubSimulatorItem(org),
      impl(new CFDSimulatorItemImpl(this, *org.impl))
{

}


CFDSimulatorItemImpl::CFDSimulatorItemImpl(CFDSimulatorItem* self, const CFDSimulatorItemImpl& org)
    : self(self)
{
    cfdBodies = org.cfdBodies;
    thrusters = org.thrusters;
    rotors = org.rotors;
}


CFDSimulatorItem::~CFDSimulatorItem()
{
    delete impl;
}


void CFDSimulatorItem::initializeClass(ExtensionManager* ext)
{
    ext->itemManager()
            .registerClass<CFDSimulatorItem>(N_("CFDSimulatorItem"))
            .addCreationPanel<CFDSimulatorItem>();
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
    this->simulatorItem = simulatorItem;

    const vector<SimulationBody*>& simBodies = simulatorItem->simulationBodies();
    for(size_t i = 0; i < simBodies.size(); ++i) {
        Body* body = simBodies[i]->body();
        createCFDBody(body);
        thrusters << body->devices();
        rotors << body->devices();
    }

    if(cfdBodies.size()) {
        simulatorItem->addPreDynamicsFunction([&](){ onPreDynamicsFunction(); });
    }
    return true;
}


void CFDSimulatorItemImpl::createCFDBody(Body* body)
{
    for(int i = 0; i < body->numLinks(); ++i) {
        CFDBody* cfdBody = new CFDBody(body);
        Link* link = body->link(i);
        CFDLink* cfdLink = cfdBody->createLink();

        Mapping& node = *link->info();
        cfdLink->link = link;
        node.read("density", cfdLink->density);
        if(!read(node, "centerOfBuoyancy", cfdLink->centerOfBuoyancy)) {
            cfdLink->centerOfBuoyancy = link->centerOfMass();
        }
        node.read("cdw", cfdLink->cdw);
        node.read("cda", cfdLink->cda);
        node.read("td", cfdLink->td);
        read(node, "surface", cfdLink->surface);
        node.read("cv", cfdLink->cv);
        cfdBodies.push_back(cfdBody);
    }
}


void CFDSimulatorItemImpl::onPreDynamicsFunction()
{
    Vector3 gravity = simulatorItem->getGravity();
    ItemList<FluidAreaItem> areaItems;
    WorldItem* worldItem = simulatorItem->findOwnerItem<WorldItem>();
    if(worldItem) {
        areaItems = worldItem->descendantItems<FluidAreaItem>();
    }

    for(size_t i = 0; i < cfdBodies.size(); ++i) {
        CFDBody* cfdBody = cfdBodies[i];
        for(int j = 0; j < cfdBody->numLinks(); ++j) {
            CFDLink* cfdLink = cfdBody->link(j);
            Link* link = cfdLink->link;
            double density = 0.0;
            double viscosity = 0.0;

            //flow
            Vector3 ff = Vector3::Zero();
            for(size_t k = 0; k <  areaItems.size(); ++k) {
                if(areaItems[k]->isCollided(link->T().translation())) {
                    density = areaItems[k]->density();
                    viscosity = areaItems[k]->viscosity();
                    ff = areaItems[k]->flow();
                }
            }

            link->f_ext() += ff;
            Vector3 cr = link->T() * Vector3(0.0, 0.0, 0.0);
            link->tau_ext() += cr.cross(ff);

            // buoyancy
            double volume = 0.0;
            if(cfdLink->density > 0.0) {
                volume = link->mass() / cfdLink->density;
            }
            Vector3 fb = density * gravity * volume * -1.0;
            link->f_ext() += fb;
            Vector3 cb = link->T() * cfdLink->centerOfBuoyancy;
            link->tau_ext() += cb.cross(fb);

            Vector3 v = link->v();
            Vector3 w = link->w();
            double cd = 0.0;
            if(density > 10.0) {
                cd = cfdLink->cdw;
            } else {
                cd = cfdLink->cda;
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
                fd[k] = 0.5 * density * v[k] * v[k] * cfdLink->surface[k * 2 + index] * cd * sign;
                double cv = cfdLink->cv;
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
                td[k] = density * w[k] * w[k] * (cfdLink->surface[index0 * 2 + index] + cfdLink->surface[index1 * 2 + index]) * cfdLink->td * sign;
                double cv = cfdLink->cv;
                tv[k] = cv * viscosity * fabs(w[k]) * sign;
            }

            link->f_ext() += fd;
            link->tau_ext() += cr.cross(fd) + td;
            link->f_ext() += fv;
            link->tau_ext() += cr.cross(fv) + tv;
        }
    }

    // Thruster
    for(int i = 0; i < thrusters.size(); ++i) {
        Thruster* thruster = thrusters[i];
        Link* link = thruster->link();
        FluidAreaItem* areaItem = nullptr;
        for(size_t j = 0; j <  areaItems.size(); ++j) {
            if(areaItems[j]->isCollided(link->T().translation())) {
                areaItem = areaItems[j];
            }
        }
        if(areaItem) {
            double density = areaItem->density();
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
    for(int i = 0; i < rotors.size(); ++i) {
        Rotor* rotor = rotors[i];
        Link* link = rotor->link();
        FluidAreaItem* areaItem = nullptr;
        for(size_t j = 0; j <  areaItems.size(); ++j) {
            if(areaItems[j]->isCollided(link->T().translation())) {
                areaItem = areaItems[j];
            }
        }
        if(areaItem) {
            double density = areaItem->density();
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


Item* CFDSimulatorItem::doDuplicate() const
{
    return new CFDSimulatorItem(*this);
}


void CFDSimulatorItem::doPutProperties(PutPropertyFunction& putProperty)
{
    SubSimulatorItem::doPutProperties(putProperty);
    impl->doPutProperties(putProperty);
}

        
void CFDSimulatorItemImpl::doPutProperties(PutPropertyFunction& putProperty)
{

}


bool CFDSimulatorItem::store(Archive& archive)
{
    SubSimulatorItem::store(archive);
    return impl->store(archive);
}


bool CFDSimulatorItemImpl::store(Archive& archive)
{
    return true;
}


bool CFDSimulatorItem::restore(const Archive& archive)
{
    SubSimulatorItem::restore(archive);
    return impl->restore(archive);
}


bool CFDSimulatorItemImpl::restore(const Archive& archive)
{
    return true;
}
