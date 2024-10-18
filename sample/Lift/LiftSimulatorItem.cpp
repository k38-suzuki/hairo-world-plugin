/**
   @author Kenta Suzuki
*/

#include "LiftSimulatorItem.h"
#include <cnoid/Archive>
#include <cnoid/Body>
#include <cnoid/BoundingBox>
#include <cnoid/ItemManager>
#include <cnoid/MathUtil>
#include <cnoid/MessageView>
#include <cnoid/SceneShape>
#include <cnoid/SimulatorItem>
#include <cnoid/WorldItem>

using namespace std;
using namespace cnoid;


void LiftSimulatorItem::initializeClass(ExtensionManager* ext)
{
    ext->itemManager()
        .registerClass<LiftSimulatorItem, SubSimulatorItem>("LiftSimulatorItem")
        .addCreationPanel<LiftSimulatorItem>();
}


LiftSimulatorItem::LiftSimulatorItem()
{
    simulatorItem = 0;
    wings.clear();
    colliders.clear();
}


LiftSimulatorItem::LiftSimulatorItem(const LiftSimulatorItem& org)
    : SubSimulatorItem(org)
{
    simulatorItem = 0;
    wings.clear();
    colliders.clear();
}


bool LiftSimulatorItem::initializeSimulation(SimulatorItem* simulatorItem)
{
    this->simulatorItem = simulatorItem;
    wings.clear();
    colliders.clear();

    const vector<SimulationBody*>& simBodies = simulatorItem->simulationBodies();
    for(auto& simBody : simBodies) {
        Body* body = simBody->body();
        wings << body->devices();
    }

    WorldItem* worldItem = simulatorItem->findOwnerItem<WorldItem>();
    if(worldItem) {
        colliders = worldItem->descendantItems<MultiColliderItem>();
    }
    for(auto& collider : colliders) {
        collider->setUnsteadyFlow(Vector3(0.0, 0.0, 0.0));
    }

    if(simBodies.size()) {
        simulatorItem->addPreDynamicsFunction([&](){ onPreDynamics(); });
    }

    return true;
}


void LiftSimulatorItem::onPreDynamics()
{
    for(auto& wing : wings) {
        Link* link = wing->link();
        Vector3 cb = link->T() * link->centerOfMass();

        MultiColliderItem* item = nullptr;
        for(auto& collider : colliders) {
            if(collision(collider, link->T().translation())) {
                item = collider;
            }
        }

        if(item) {
            double density = item->density();
            const BoundingBox& bb = link->visualShape()->boundingBox();
            // double s = wing->wingspan() * wing->chordLength();
            double s = bb.size().x() * bb.size().y();
            double cl = wing->cl();
            Vector3 v = link->v();
            double v_norm = v.dot(v);
            double v2 = v_norm * v_norm;
            Vector3 v_local = link->R().inverse() * v;
            double aoa = -atan2(v_local.z(), v_local.x()) * TO_DEGREE;
            cl = aoa * 0.1;

            Vector3 unit_x = Vector3::UnitX();
            Vector3 unit_z = Vector3::UnitZ();

            Vector3 a = link->R() * unit_x;
            Vector3 b = link->R() * unit_z;

            if(wing->on()) {
                if(a.dot(v) > 0.0) {
                    Vector3 n = v.cross(b).cross(v).normalized();
                    Vector3 lift = 0.5 * cl * density * v2 * s * n;

                    link->f_ext() += lift;
                    link->tau_ext() += cb.cross(lift);
                }
            }
        }
    }
}


Item* LiftSimulatorItem::doCloneItem(CloneMap* cloneMap) const
{
    return new LiftSimulatorItem(*this);
}


void LiftSimulatorItem::doPutProperties(PutPropertyFunction& putProperty)
{
    SubSimulatorItem::doPutProperties(putProperty);
}


bool LiftSimulatorItem::store(Archive& archive)
{
    if(!SubSimulatorItem::store(archive)) {
        return false;
    }
    return true;
}


bool LiftSimulatorItem::restore(const Archive& archive)
{
    if(!SubSimulatorItem::restore(archive)) {
        return false;
    }
    return true;
}
