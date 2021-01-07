/**
   \file
   \author Kenta Suzuki
*/

#include "Rotor.h"
#include <cnoid/Archive>
#include <cnoid/EigenUtil>
#include <cnoid/MeshGenerator>
#include <cnoid/Link>
#include <cnoid/SceneDevice>
#include <cnoid/SceneMarkers>
#include <cnoid/SceneShape>
#include <cnoid/YAMLBodyLoader>

using namespace cnoid;

bool readRotor(YAMLBodyLoader& loader, Mapping& node)
{
    RotorPtr rotor = new Rotor;
    double f;
    bool r;
    if(node.read("forceOffset", f)) rotor->setForceOffset(f);
    if(node.read("torqueOffset", f)) rotor->setTorqueOffset(f);
    if(node.read("k", f)) rotor->setK(f);
    if(node.read("kv", f)) rotor->setKv(f);
    if(node.read("diameter", f)) rotor->setDiameter(f);
    if(node.read("pitch", f)) rotor->setPitch(f);
    if(node.read("reverse", r)) rotor->setReverse(r);
    return loader.readDevice(rotor, node);
}


namespace cnoid {

class SceneRotor : public SceneDevice
{
public:
    SceneRotor(Device* device);

private:
    void updateScene();
    Rotor* rotorDevice;
    SgPosTransformPtr scene;
    bool isRotorAttached;
};

}


SceneRotor::SceneRotor(Device* device)
    : SceneDevice(device)
{
    MeshGenerator generator;
    rotorDevice = static_cast<Rotor*>(device);
    scene = new SgPosTransform();
    SgShape* shape = new SgShape();
    shape->setMesh(generator.generateArrow(0.01, 0.05, 0.02, 0.05));
    shape->setName(device->name());
    SgMaterial* material = new SgMaterial();
    material->setDiffuseColor(Vector3(0.0, 1.0, 0.0));
    material->setTransparency(0.5);
    shape->setMaterial(material);
    scene->addChild(shape);
    Link* link = rotorDevice->link();
    Vector3 translation(0.0, 0.0, 0.025);
    Matrix3 rotation = rotFromRpy(Vector3(90.0, 0.0, 0.0) * TO_RADIAN);
    scene->setTranslation(translation);
    scene->setRotation(rotation);
    isRotorAttached = false;
    setFunctionOnStateChanged([&](){ updateScene(); });
}


void SceneRotor::updateScene()
{
    bool on = rotorDevice->on();
    if(on != isRotorAttached) {
        if(on) {
            addChildOnce(scene);
        }
        else {
            removeChild(scene);
        }
        isRotorAttached = on;
    }

//    scene->notifyUpdate();
}


SceneDevice* createSceneRotor(Device* device)
{
    return new SceneRotor(device);
}


struct RotorRegistration
{
    RotorRegistration() {
        YAMLBodyLoader::addNodeType("Rotor", readRotor);
        SceneDevice::registerSceneDeviceFactory<Rotor>(createSceneRotor);
    }
} registrationRotor;


Rotor::Rotor()
{
    on_ = true;
    force_ = 0.0;
    torque_ = 0.0;
    forceOffset_ = 0.0;
    torqueOffset_ = 0.0;
    k_ = 22.0;
    kv_ = 0.0;
    diameter_ = 0.0;
    pitch_ = 0.0;
    voltage_ = 0.0;
    reverse_ = false;
}


Rotor::Rotor(const Rotor& org, bool copyStateOnly)
    : Device(org, copyStateOnly)
{
    copyStateFrom(org);
}


const char* Rotor::typeName() const
{
    return "Rotor";
}


void Rotor::copyStateFrom(const Rotor& other)
{
    on_ = other.on_;
    force_ = other.force_;
    torque_ = other.torque_;
    forceOffset_ = other.forceOffset_;
    torqueOffset_ = other.torqueOffset_;
    k_ = other.k_;
    kv_ = other.kv_;
    diameter_ = other.diameter_;
    pitch_ = other.pitch_;
    voltage_ = other.voltage_;
    reverse_ = other.reverse_;
}


void Rotor::copyStateFrom(const DeviceState& other)
{
    if(typeid(other) != typeid(Rotor)){
        throw std::invalid_argument("Type mismatch in the Device::copyStateFrom function");
    }
    copyStateFrom(static_cast<const Rotor&>(other));
}


DeviceState* Rotor::cloneState() const
{
    return new Rotor(*this, true);
}


Referenced* Rotor::doClone(CloneMap*) const
{
    return new Rotor(*this);
}


void Rotor::forEachActualType(std::function<bool(const std::type_info& type)> func)
{
    if(!func(typeid(Rotor))){
        Device::forEachActualType(func);
    }
}


void Rotor::clearState()
{
    force_ = 0.0;
    torque_ = 0.0;
    voltage_ = 0.0;
}


bool Rotor::on() const
{
    return on_;
}


void Rotor::on(bool on)
{
    on_ = on;
}


int Rotor::stateSize() const
{
    return 10;
}


const double* Rotor::readState(const double* buf)
{
    int i = 0;
    on_ = buf[i++];
    force_ = buf[i++];
    torque_ = buf[i++];
    forceOffset_ = buf[i++];
    torqueOffset_ = buf[i++];
    k_ = buf[i++];
    kv_ = buf[i++];
    diameter_ = buf[i++];
    pitch_ = buf[i++];
    voltage_ = buf[i++];
    reverse_= buf[i++];
    return buf + i;
}


double* Rotor::writeState(double* out_buf) const
{
    int i = 0;
    out_buf[i++] = on_ ? 1.0 : 0.0;
    out_buf[i++] = force_;
    out_buf[i++] = torque_;
    out_buf[i++] = forceOffset_;
    out_buf[i++] = torqueOffset_;
    out_buf[i++] = k_;
    out_buf[i++] = kv_;
    out_buf[i++] = diameter_;
    out_buf[i++] = pitch_;
    out_buf[i++] = voltage_;
    out_buf[i++] = reverse_;
    return out_buf + i;
}
