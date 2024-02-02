/**
   @author Kenta Suzuki
*/

#include "WingDevice.h"
#include <cnoid/Archive>
#include <cnoid/BoundingBox>
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

class SceneWingDevice : public SceneDevice
{
public:
    SceneWingDevice(Device* device);

private:
    void onStateChanged();
    WingDevice* wingDevice;
    SgPosTransformPtr scene;
    bool isWingDeviceAttached;
};

}


SceneWingDevice::SceneWingDevice(Device* device)
    : SceneDevice(device)
{
    MeshGenerator generator;
    wingDevice = static_cast<WingDevice*>(device);
    if(!scene) {
        scene = new SgPosTransform;
        SgShape* shape = new SgShape;
        Link* link = wingDevice->link();
        double s = wingDevice->wingspan();
        double c = wingDevice->chordLength();
        const BoundingBox& bb = link->visualShape()->boundingBox();
        // shape->setMesh(generator.generateBox(Vector3(c, s, 0.01)));
        shape->setMesh(generator.generateBox(bb.size() + Vector3(0.01, 0.01, 0.01)));
        shape->setName(device->name());
        SgMaterial* material = shape->getOrCreateMaterial();
        material->setDiffuseColor(Vector3(1.0, 0.0, 0.0));
        material->setTransparency(0.5);
        scene->addChild(shape);
    }
    isWingDeviceAttached = false;
    setFunctionOnStateChanged([&](){ onStateChanged(); });
}


void SceneWingDevice::onStateChanged()
{
    bool on = wingDevice->on();
    bool symbol = wingDevice->symbol();
    if(on != isWingDeviceAttached && symbol) {
        if(on) {
            addChildOnce(scene);
        } else {
            removeChild(scene);
        }
        isWingDeviceAttached = on;
    }
}


SceneDevice* createSceneWingDevice(Device* device)
{
    return new SceneWingDevice(device);
}


WingDevice::WingDevice()
    : spec(new Spec)
{
    on_ = true;
    cl_ = 0.0;
    wingspan_ = 1.0;
    chordLength_ = 1.0;
    symbol_ = true;
}


WingDevice::WingDevice(const WingDevice& org, bool copyStateOnly)
    : Device(org, copyStateOnly)
{
    copyStateFrom(org);

    if(!copyStateOnly) {
        spec.reset(new Spec);
        if(org.spec) {

        } else {

        }
    }
}


const char* WingDevice::typeName() const
{
    return "WingDevice";
}


void WingDevice::copyStateFrom(const WingDevice& other)
{
    copyWingDeviceStateFrom(other);
}


void WingDevice::copyStateFrom(const DeviceState& other)
{
    if(typeid(other) != typeid(WingDevice)) {
        throw std::invalid_argument("Type mismatch in the Device::copyStateFrom function");
    }
    copyStateFrom(static_cast<const WingDevice&>(other));
}


DeviceState* WingDevice::cloneState() const
{
    return new WingDevice(*this, true);
}


void WingDevice::forEachActualType(std::function<bool(const std::type_info& type)> func)
{
    if(!func(typeid(WingDevice))) {
        Device::forEachActualType(func);
    }
}


void WingDevice::clearState()
{

}


int WingDevice::stateSize() const
{
    return 5;
}


bool WingDevice::on() const
{
    return on_;
}


void WingDevice::on(bool on)
{
    on_ = on;
}


const double* WingDevice::readState(const double* buf)
{
    on_ = buf[0];
    cl_ = buf[1];
    wingspan_ = buf[2];
    chordLength_ = buf[3];
    symbol_ = buf[4];
    return buf + 5;
}


double* WingDevice::writeState(double* out_buf) const
{
    out_buf[0] = on_ ? 1.0 : 0.0;
    out_buf[1] = cl_;
    out_buf[2] = wingspan_;
    out_buf[3] = chordLength_;
    out_buf[4] = symbol_;
    return out_buf + 5;
}


bool WingDevice::readSpecifications(const Mapping* info)
{
    info->read("on", on_);
    info->read("cl", cl_);
    info->read("wingspan", wingspan_);
    info->read("chord_length", chordLength_);
    info->read("symbol", symbol_);
    return true;
}


bool WingDevice::writeSpecifications(Mapping* info) const
{
    info->write("on", on_);
    info->write("cl", cl_);
    info->write("wingspan", wingspan_);
    info->write("chord_length", chordLength_);
    info->write("symbol", symbol_);
    return true;
}


Referenced* WingDevice::doClone(CloneMap*) const
{
    return new WingDevice(*this, false);
}


void WingDevice::copyWingDeviceStateFrom(const WingDevice& other)
{
    on_ = other.on_;
    cl_ = other.cl_;
    wingspan_ = other.wingspan_;
    chordLength_ = other.chordLength_;
    symbol_ = other.symbol_;
}


namespace {

bool readWingDevice(StdBodyLoader* loader, const Mapping* info)
{
    WingDevicePtr wing = new WingDevice;
    if(!wing->readSpecifications(info)) {
        wing.reset();
    }

    bool result = false;
    if(wing) {
        result = loader->readDevice(wing, info);
    }
    return result;
}


struct WingDeviceRegistration
{
    WingDeviceRegistration() {
        StdBodyLoader::registerNodeType("WingDevice", readWingDevice);
        StdBodyWriter::registerDeviceWriter<WingDevice>(
                    "WingDevice",
                    [](StdBodyWriter* /* writer */, Mapping* info, const WingDevice* wing) {
            return wing->writeSpecifications(info);
        });
        SceneDevice::registerSceneDeviceFactory<WingDevice>(createSceneWingDevice);
    }
} registrationWingDevice;

}
