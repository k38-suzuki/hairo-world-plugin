/**
   \file
   \author Kenta Suzuki
*/

#include "Thruster.h"
#include <cnoid/Archive>
#include <cnoid/EigenUtil>
#include <cnoid/MeshGenerator>
#include <cnoid/Link>
#include <cnoid/SceneDevice>
#include <cnoid/SceneMarkers>
#include <cnoid/SceneShape>
#include <cnoid/YAMLBodyLoader>

using namespace cnoid;

bool readThruster(YAMLBodyLoader& loader, Mapping& node)
{
    ThrusterPtr thruster = new Thruster;
    double f;
    bool b;
    if(node.read("forceOffset", f)) thruster->setForceOffset(f);
    if(node.read("torqueOffset", f)) thruster->setTorqueOffset(f);
    if(node.read("symbol", b)) thruster->setSymbol(b);
    return loader.readDevice(thruster, node);
}


namespace cnoid {

class SceneThruster : public SceneDevice
{
public:
    SceneThruster(Device* device);

private:
    void updateScene();
    Thruster* thrusterDevice;
    SgPosTransformPtr scene;
    bool isThrusterAttached;
};

}


SceneThruster::SceneThruster(Device* device)
    : SceneDevice(device)
{
    MeshGenerator generator;
    thrusterDevice = static_cast<Thruster*>(device);
    scene = new SgPosTransform();
    SgShape* shape = new SgShape();
    shape->setMesh(generator.generateArrow(0.01, 0.05, 0.02, 0.05));
    shape->setName(device->name());
    SgMaterial* material = new SgMaterial();
    material->setDiffuseColor(Vector3(1.0, 0.0, 0.0));
    material->setTransparency(0.5);
    shape->setMaterial(material);
    scene->addChild(shape);
    Link* link = thrusterDevice->link();
    Vector3 translation(0.025, 0.0, 0.0);
    Matrix3 rotation = rotFromRpy(Vector3(0.0, 0.0, -90.0) * TO_RADIAN);
    scene->setTranslation(translation);
    scene->setRotation(rotation);
    isThrusterAttached = false;
    setFunctionOnStateChanged([&](){ updateScene(); });
}


void SceneThruster::updateScene()
{
    bool on = thrusterDevice->on();
    bool symbol = thrusterDevice->symbol();
    if(on != isThrusterAttached && symbol) {
        if(on) {
            addChildOnce(scene);
        } else {
            removeChild(scene);
        }
        isThrusterAttached = on;
    }

//    scene->notifyUpdate();
}


SceneDevice* createSceneThruster(Device* device)
{
    return new SceneThruster(device);
}


struct ThrusterRegistration
{
    ThrusterRegistration() {
        YAMLBodyLoader::addNodeType("Thruster", readThruster);
        SceneDevice::registerSceneDeviceFactory<Thruster>(createSceneThruster);
    }
} registrationThruster;


Thruster::Thruster()
{
    on_ = true;
    force_ = 0.0;
    torque_ = 0.0;
    forceOffset_ = 0.0;
    torqueOffset_ = 0.0;
    symbol_ = true;
}


Thruster::Thruster(const Thruster& org, bool copyStateOnly)
    : Device(org, copyStateOnly)
{
    copyStateFrom(org);
}


const char* Thruster::typeName() const
{
    return "Thruster";
}


void Thruster::copyStateFrom(const Thruster& other)
{
    on_ = other.on_;
    force_ = other.force_;
    torque_ = other.torque_;
    forceOffset_ = other.forceOffset_;
    torqueOffset_ = other.torqueOffset_;
    symbol_ = other.symbol_;
}


void Thruster::copyStateFrom(const DeviceState& other)
{
    if(typeid(other) != typeid(Thruster)){
        throw std::invalid_argument("Type mismatch in the Device::copyStateFrom function");
    }
    copyStateFrom(static_cast<const Thruster&>(other));
}


DeviceState* Thruster::cloneState() const
{
    return new Thruster(*this, true);
}


Referenced* Thruster::doClone(CloneMap*) const
{
    return new Thruster(*this);
}


void Thruster::forEachActualType(std::function<bool(const std::type_info& type)> func)
{
    if(!func(typeid(Thruster))){
        Device::forEachActualType(func);
    }
}


void Thruster::clearState()
{
    force_ = 0.0;
    torque_ = 0.0;
}


bool Thruster::on() const
{
    return on_;
}


void Thruster::on(bool on)
{
    on_ = on;
}


int Thruster::stateSize() const
{
    return 3;
}


const double* Thruster::readState(const double* buf)
{
    int i = 0;
    on_ = buf[i++];
    force_ = buf[i++];
    torque_ = buf[i++];
    forceOffset_ = buf[i++];
    torqueOffset_ = buf[i++];
    symbol_ = buf[i++];
    return buf + i;
}


double* Thruster::writeState(double* out_buf) const
{
    int i = 0;
    out_buf[i++] = on_ ? 1.0 : 0.0;
    out_buf[i++] = force_;
    out_buf[i++] = torque_;
    out_buf[i++] = forceOffset_;
    out_buf[i++] = torqueOffset_;
    out_buf[i++] = symbol_ ? 1.0 : 0.0;
    return out_buf + i;
}
