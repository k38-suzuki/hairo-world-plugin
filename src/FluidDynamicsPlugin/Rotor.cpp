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
#include <cnoid/StdBodyLoader>
#include <cnoid/StdBodyWriter>

using namespace cnoid;

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
    bool symbol = rotorDevice->symbol();
    if(on != isRotorAttached && symbol) {
        if(on) {
            addChildOnce(scene);
        } else {
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


Rotor::Rotor()
{
    setDirection(Vector3(0.0, 0.0, 1.0));
    k_ = 22.0;
    kv_ = 0.0;
    diameter_ = 0.0;
    pitch_ = 0.0;
    voltage_ = 0.0;
    reverse_ = false;
}


Rotor::Rotor(const Rotor& org, bool copyStateOnly)
    : Thruster(org, copyStateOnly)
{
    copyRotorStateFrom(org);
}


const char* Rotor::typeName() const
{
    return "Rotor";
}


void Rotor::copyStateFrom(const Rotor& other)
{
    Thruster::copyStateFrom(other);
    copyRotorStateFrom(other);
}


void Rotor::copyStateFrom(const DeviceState& other)
{
    if(typeid(other) != typeid(Rotor)){
        throw std::invalid_argument("Type mismatch in the Device::copyStateFrom function");
    }
    copyStateFrom(static_cast<const Rotor&>(other));
}


void Rotor::copyRotorStateFrom(const Rotor& other)
{
    k_ = other.k_;
    kv_ = other.kv_;
    diameter_ = other.diameter_;
    pitch_ = other.pitch_;
    voltage_ = other.voltage_;
    reverse_ = other.reverse_;
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
        Thruster::forEachActualType(func);
    }
}


void Rotor::clearState()
{
    Thruster::clearState();
    voltage_ = 0.0;
}


int Rotor::stateSize() const
{
    return Thruster::stateSize() + 6;
}


const double* Rotor::readState(const double* buf)
{
    int i = Thruster::stateSize();
    Thruster::readState(buf);
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
    int i = Thruster::stateSize();
    Thruster::writeState(out_buf);
    out_buf[i++] = k_;
    out_buf[i++] = kv_;
    out_buf[i++] = diameter_;
    out_buf[i++] = pitch_;
    out_buf[i++] = voltage_;
    out_buf[i++] = reverse_;
    return out_buf + i;
}


bool Rotor::readSpecifications(const Mapping* info)
{
    if(!Thruster::readSpecifications(info)) {
        return false;
    }

    info->read("k", k_);
    info->read("kv", kv_);
    info->read("diameter", diameter_);
    info->read("pitch", pitch_);
    info->read("reverse", reverse_);

    return true;
}


bool Rotor::writeSpecifications(Mapping* info) const
{
    if(!Thruster::writeSpecifications(info)) {
        return false;
    }

    info->write("k", k_);
    info->write("kv", kv_);
    info->write("diameter", diameter_);
    info->write("pitch", pitch_);
    info->write("reverse", reverse_);

    return true;
}


namespace {

bool readRotor(StdBodyLoader* loader, const Mapping* info)
{
    RotorPtr rotor = new Rotor;
    if(!rotor->readSpecifications(info)) {
        rotor.reset();
    }

    bool result = false;
    if(rotor) {
        result = loader->readDevice(rotor, info);
    }
    return result;
}


struct RotorRegistration
{
    RotorRegistration() {
        StdBodyLoader::registerNodeType("Rotor", readRotor);
        StdBodyWriter::registerDeviceWriter<Rotor>("Rotor",
                                                   [](StdBodyWriter* /* writer */, Mapping* info, const Rotor* rotor){
            return rotor->writeSpecifications(info);
        });
        SceneDevice::registerSceneDeviceFactory<Rotor>(createSceneRotor);
    }
} registrationRotor;

}
