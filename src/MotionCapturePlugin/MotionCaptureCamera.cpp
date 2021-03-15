/**
   \file
   \author Kenta Suzuki
*/

#include "MotionCaptureCamera.h"
#include <cnoid/EigenUtil>
#include <cnoid/SceneDevice>
#include <cnoid/SceneShape>
#include <cnoid/YAMLBodyLoader>
#include <cnoid/YAMLReader>

using namespace cnoid;

bool readMotionCaptureCamera(YAMLBodyLoader& loader, Mapping& node)
{
    MotionCaptureCameraPtr camera = new MotionCaptureCamera;
    int f;
    if(node.read("fieldOfView", f)) camera->setFieldOfView(f);
    double fl;
    if(node.read("focalLength", fl)) camera->setFocalLength(fl);
    auto& a = *node.findListing("aspectRatio");
    if(a.size() == 2) {
        camera->setAspectRatio(Vector2(a[0].toDouble(), a[1].toDouble()));
    }
    auto& b = *node.findListing("diffuseColor");
    if(b.size() == 3) {
        camera->setDiffuseColor(Vector3(b[0].toDouble(), b[1].toDouble(), b[2].toDouble()));
    }
    auto& c = *node.findListing("emissiceColor");
    if(c.size() == 3) {
        camera->setEmissiveColor(Vector3(c[0].toDouble(), c[1].toDouble(), c[2].toDouble()));
    }
    auto& d = *node.findListing("specularColor");
    if(d.size() == 3) {
        camera->setSpecularColor(Vector3(d[0].toDouble(), d[1].toDouble(), d[2].toDouble()));
    }
    float s;
    if(node.read("ambientIntensity", s)) camera->setAmbientIntensity(s);
    if(node.read("shininess", s)) camera->setShininess(s);
    if(node.read("transparency", s)) camera->setTransparency(s);
    return loader.readDevice(camera, node);
}


namespace cnoid {

class SceneMotionCaptureCamera : public SceneDevice
{
public:
    SceneMotionCaptureCamera(Device* device);

private:
    void updateScene();
    MotionCaptureCamera* camera;
    SgPosTransform* transform;
    bool isMotionCaptureCameraAttached;
};

SceneMotionCaptureCamera::SceneMotionCaptureCamera(Device* device)
    : SceneDevice(device)
{
    camera = static_cast<MotionCaptureCamera*>(device);
    transform = new SgPosTransform();
    SgShape* shape = new SgShape();
    SgMesh* mesh = shape->setMesh(new SgMesh());

    SgVertexArray& vertices = *mesh->setVertices(new SgVertexArray());
    vertices.resize(5);
    double rangehy = camera->focalLength() * tan((double)camera->fieldOfView() / 2.0 * TO_RADIAN);
    double rangehz = rangehy * 2.0 / camera->aspectRatio()[0] * camera->aspectRatio()[1] / 2.0;
    vertices[0] << 0.0, 0.0, 0.0;
    vertices[1] << camera->focalLength(), rangehy,  rangehz;
    vertices[2] << camera->focalLength(), rangehy, -rangehz;
    vertices[3] << camera->focalLength(), -rangehy, -rangehz;
    vertices[4] << camera->focalLength(), -rangehy,  rangehz;

    mesh->setNumTriangles(6);
    mesh->setTriangle(0, 0, 2, 1);
    mesh->setTriangle(1, 0, 3, 2);
    mesh->setTriangle(2, 0, 4, 3);
    mesh->setTriangle(3, 0, 1, 4);
    mesh->setTriangle(4, 1, 2, 3);
    mesh->setTriangle(5, 1, 3, 4);

    SgMaterial* material = new SgMaterial();
    float s = 127.0f * std::max(0.0f, std::min((float)camera->shininess(), 1.0f)) + 1.0f;
    material->setDiffuseColor(camera->diffuseColor());
    material->setEmissiveColor(camera->emissiveColor());
    material->setSpecularColor(camera->specularColor());
    material->setAmbientIntensity(camera->ambientIntensity());
    material->setSpecularExponent(s);
    material->setTransparency(camera->transparency());
    shape->setMaterial(material);
    transform->addChild(shape);
    camera->setName(device->name());
    isMotionCaptureCameraAttached = false;
    setFunctionOnStateChanged([&](){ updateScene(); });
}


void SceneMotionCaptureCamera::updateScene()
{
    bool on = camera->on();
    if(on != isMotionCaptureCameraAttached) {
        if(on) {
            addChildOnce(transform);
        } else {
            removeChild(transform);
        }
        isMotionCaptureCameraAttached = on;
    }
}

}


SceneDevice* createSceneMotionCaptureCamera(Device* device)
{
    return new SceneMotionCaptureCamera(device);
}


struct MotionCaptureCameraRegistration
{
    MotionCaptureCameraRegistration() {
        YAMLBodyLoader::addNodeType("MotionCaptureCamera", readMotionCaptureCamera);
        SceneDevice::registerSceneDeviceFactory<MotionCaptureCamera>(createSceneMotionCaptureCamera);
    }
} registrationMotionCaptureCamera;


MotionCaptureCamera::MotionCaptureCamera()
{
    on_ = true;
    fieldOfView_ = 65.0;
    focalLength_ = 10.0;
    aspectRatio_ = Vector2(16.0, 9.0);
    diffuseColor_ = Vector3(1.0, 0.0, 0.0);
    emissiveColor_ = Vector3(0.0, 0.0, 0.0);
    specularColor_ = Vector3(0.0, 0.0, 0.0);
    ambientIntensity_ = 0.0;
    shininess_ = 0.0;
    transparency_ = 0.0;
}


MotionCaptureCamera::MotionCaptureCamera(const MotionCaptureCamera& org, bool copyStateOnly)
    : Device(org, copyStateOnly)
{
    copyStateFrom(org);
}


const char* MotionCaptureCamera::typeName() const
{
    return "MotionCaptureCamera";
}


void MotionCaptureCamera::copyStateFrom(const MotionCaptureCamera& other)
{
    on_ = other.on_;
    fieldOfView_ = other.fieldOfView_;
    focalLength_ = other.focalLength_;
    aspectRatio_ = other.aspectRatio_;
    diffuseColor_ = other.diffuseColor_;
    emissiveColor_ = other.emissiveColor_;
    specularColor_ = other.specularColor_;
    ambientIntensity_ = other.ambientIntensity_;
    shininess_ = other.shininess_;
    transparency_ = other.transparency_;
}


void MotionCaptureCamera::copyStateFrom(const DeviceState& other)
{
    if(typeid(other) != typeid(MotionCaptureCamera)){
        throw std::invalid_argument("Type mismatch in the Device::copyStateFrom function");

    }
    copyStateFrom(static_cast<const MotionCaptureCamera&>(other));

}


DeviceState* MotionCaptureCamera::cloneState() const
{
    return new MotionCaptureCamera(*this, false);
}


Referenced* MotionCaptureCamera::doClone(CloneMap*) const
{
    return new MotionCaptureCamera(*this);
}


void MotionCaptureCamera::forEachActualType(std::function<bool(const std::type_info& type)> func)
{
    if(!func(typeid(MotionCaptureCamera))){
        Device::forEachActualType(func);

    }
}


int MotionCaptureCamera::stateSize() const
{
    return 4;
}


const double* MotionCaptureCamera::readState(const double* buf)
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


double* MotionCaptureCamera::writeState(double* out_buf) const
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


void MotionCaptureCamera::on(const bool on)
{
    on_ = on;
}


bool MotionCaptureCamera::on() const
{
    return on_;
}
