/**
   @author Kenta Suzuki
*/

#include "ScopeDevice.h"
#include <cnoid/EigenArchive>
#include <cnoid/EigenUtil>
#include <cnoid/SceneDevice>
#include <cnoid/SceneShape>
#include <cnoid/StdBodyLoader>
#include <cnoid/StdBodyWriter>
#include <cnoid/YAMLReader>

using namespace cnoid;

namespace cnoid {

class SceneScopeDevice : public SceneDevice
{
public:
    SceneScopeDevice(Device* device);

private:
    void updateScene();
    ScopeDevice* scope;
    SgPosTransform* transform;
    bool isScopeDeviceAttached;
};


SceneScopeDevice::SceneScopeDevice(Device* device)
    : SceneDevice(device)
{
    scope = static_cast<ScopeDevice*>(device);
    transform = new SgPosTransform;
    SgShape* shape = new SgShape;
    SgMesh* mesh = shape->setMesh(new SgMesh);

    SgVertexArray& vertices = *mesh->setVertices(new SgVertexArray);
    vertices.resize(5);
    double rangehy = scope->focalLength() * tan((double)scope->fieldOfView() / 2.0 * TO_RADIAN);
    double rangehz = rangehy * 2.0 / scope->aspectRatio()[0] * scope->aspectRatio()[1] / 2.0;
    vertices[0] << 0.0, 0.0, 0.0;
    vertices[1] << scope->focalLength(), rangehy,  rangehz;
    vertices[2] << scope->focalLength(), rangehy, -rangehz;
    vertices[3] << scope->focalLength(), -rangehy, -rangehz;
    vertices[4] << scope->focalLength(), -rangehy,  rangehz;

    mesh->setNumTriangles(6);
    mesh->setTriangle(0, 0, 2, 1);
    mesh->setTriangle(1, 0, 3, 2);
    mesh->setTriangle(2, 0, 4, 3);
    mesh->setTriangle(3, 0, 1, 4);
    mesh->setTriangle(4, 1, 2, 3);
    mesh->setTriangle(5, 1, 3, 4);

    SgMaterial* material = new SgMaterial;
    float s = 127.0f * std::max(0.0f, std::min((float)scope->shininess(), 1.0f)) + 1.0f;
    material->setDiffuseColor(scope->diffuseColor());
    material->setAmbientIntensity(scope->ambientIntensity());
    material->setSpecularExponent(s);
    material->setTransparency(scope->transparency());
    shape->setMaterial(material);
    transform->addChild(shape);
    scope->setName(device->name());
    isScopeDeviceAttached = false;
    setFunctionOnStateChanged([&](){ updateScene(); });
}


void SceneScopeDevice::updateScene()
{
    bool on = scope->on();
    if(on != isScopeDeviceAttached) {
        if(on) {
            addChildOnce(transform);
        } else {
            removeChild(transform);
        }
        isScopeDeviceAttached = on;
    }
}

}


SceneDevice* createSceneScopeDevice(Device* device)
{
    return new SceneScopeDevice(device);
}


ScopeDevice::ScopeDevice()
    : Device()
{
    on_ = true;
    fieldOfView_ = 65.0;
    focalLength_ = 10.0;
    aspectRatio_ << 16.0, 9.0;
    diffuseColor_ << 1.0, 0.0, 0.0;
    ambientIntensity_ = 0.0;
    shininess_ = 0.0;
    transparency_ = 0.0;
}


ScopeDevice::ScopeDevice(const ScopeDevice& org, bool copyStateOnly)
    : Device(org, copyStateOnly)
{
    copyStateFrom(org);
}


const char* ScopeDevice::typeName() const
{
    return "ScopeDevice";
}


void ScopeDevice::copyStateFrom(const ScopeDevice& other)
{
    on_ = other.on_;
    fieldOfView_ = other.fieldOfView_;
    focalLength_ = other.focalLength_;
    aspectRatio_ = other.aspectRatio_;
    diffuseColor_ = other.diffuseColor_;
    ambientIntensity_ = other.ambientIntensity_;
    shininess_ = other.shininess_;
    transparency_ = other.transparency_;
}


void ScopeDevice::copyStateFrom(const DeviceState& other)
{
    if(typeid(other) != typeid(ScopeDevice)) {
        throw std::invalid_argument("Type mismatch in the Device::copyStateFrom function");

    }
    copyStateFrom(static_cast<const ScopeDevice&>(other));

}


DeviceState* ScopeDevice::cloneState() const
{
    return new ScopeDevice(*this, false);
}


Referenced* ScopeDevice::doClone(CloneMap*) const
{
    return new ScopeDevice(*this);
}


void ScopeDevice::forEachActualType(std::function<bool(const std::type_info& type)> func)
{
    if(!func(typeid(ScopeDevice))) {
        Device::forEachActualType(func);

    }
}


void ScopeDevice::on(bool on)
{
    on_ = on;
}


bool ScopeDevice::on() const
{
    return on_;
}


int ScopeDevice::stateSize() const
{
    return 4;
}


const double* ScopeDevice::readState(const double* buf)
{
    int i = 0;
    on_ = buf[i++];
    fieldOfView_ = buf[i++];
    focalLength_ = buf[i++];
    aspectRatio_[0] = buf[i++];
    aspectRatio_[1] = buf[i++];
    diffuseColor_[0] = buf[i++];
    diffuseColor_[1] = buf[i++];
    diffuseColor_[2] = buf[i++];
    return buf + i;
}


double* ScopeDevice::writeState(double* out_buf) const
{
    int i = 0;
    out_buf[i++] = on_ ? 1.0 : 0.0;
    out_buf[i++] = fieldOfView_;
    out_buf[i++] = focalLength_;
    out_buf[i++] = aspectRatio_[0];
    out_buf[i++] = aspectRatio_[1];
    out_buf[i++] = diffuseColor_[0];
    out_buf[i++] = diffuseColor_[1];
    out_buf[i++] = diffuseColor_[2];
    return out_buf + i;
}


bool ScopeDevice::readSpecifications(const Mapping* info)
{
    info->read("fieldOfView", fieldOfView_);
    info->read("focalLength", focalLength_);
    read(info, "aspectRatio", aspectRatio_);
    read(info, "diffuseColor", diffuseColor_);
    info->read("ambientIntensity", ambientIntensity_);
    info->read("shininess", shininess_);
    info->read("transparency", transparency_);
    return true;
}


bool ScopeDevice::writeSpecifications(Mapping* info) const
{
    info->write("fieldOfView", fieldOfView_);
    info->write("focalLength", focalLength_);
    write(info, "aspectRatio", aspectRatio_);
    write(info, "diffuseColor", diffuseColor_);
    info->write("ambientIntensity", ambientIntensity_);
    info->write("shininess", shininess_);
    info->write("transparency", transparency_);
    return true;
}


namespace {

bool readScopeDevice(StdBodyLoader* loader, const Mapping* info)
{
    ScopeDevicePtr scope = new ScopeDevice;
    if(!scope->readSpecifications(info)) {
        scope.reset();
    }

    bool result = false;
    if(scope) {
        result = loader->readDevice(scope, info);
    }
    return result;
}


struct ScopeDeviceRegistration
{
    ScopeDeviceRegistration() {
        StdBodyLoader::registerNodeType("ScopeDevice", readScopeDevice);
        StdBodyWriter::registerDeviceWriter<ScopeDevice>("ScopeDevice",
                                                         [](StdBodyWriter* /* writer */, Mapping* info, const ScopeDevice* scope) {
            return scope->writeSpecifications(info);
        });
        SceneDevice::registerSceneDeviceFactory<ScopeDevice>(createSceneScopeDevice);
    }
} registrationScopeDevice;

}
