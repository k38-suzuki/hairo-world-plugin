/**
   @author Kenta Suzuki
*/

#include "DoseMeter.h"
#include <cnoid/BoundingBox>
#include <cnoid/EigenArchive>
#include <cnoid/Link>
#include <cnoid/MeshGenerator>
#include <cnoid/SceneDevice>
#include <cnoid/SceneDrawables>
#include <cnoid/StdBodyFileUtil>

using namespace std;
using namespace cnoid;

namespace cnoid {

class SceneDoseMeter : public SceneDevice
{
public:
    SceneDoseMeter(Device* device);

private:
    void onStateChanged();
    DoseMeter* doseMeter;
    SgPosTransformPtr scene;
    bool isDoseMeterAttached;
};

}


SceneDoseMeter::SceneDoseMeter(Device* device)
    : SceneDevice(device)
{
    MeshGenerator generator;
    doseMeter = static_cast<DoseMeter*>(device);
    if(!scene) {
        scene = new SgPosTransform;
        Link* link = doseMeter->link();
        SgShape* shape = new SgShape;
        const BoundingBox& bb = link->visualShape()->boundingBox();
        shape->setMesh(generator.generateBox(bb.size() + Vector3(0.01, 0.01, 0.01)));
        shape->setName(doseMeter->name());
        SgMaterial* material = shape->getOrCreateMaterial();
        material->setDiffuseColor(doseMeter->color());
        material->setTransparency(0.5);
        scene->setTranslation(bb.center());
        scene->addChild(shape);
    }
    isDoseMeterAttached = false;
    setFunctionOnStateChanged([&](){ onStateChanged(); });
}


void SceneDoseMeter::onStateChanged()
{
    bool on = doseMeter->on();
    if(on != isDoseMeterAttached) {
        if(on) {
            addChildOnce(scene);
        } else {
            removeChild(scene);
        }
        isDoseMeterAttached = on;
    }

    SgShape* shape = dynamic_cast<SgShape*>(scene->child(0));
    if(shape) {
        SgMaterial* material = shape->getOrCreateMaterial();
        material->setDiffuseColor(doseMeter->color());
        material->notifyUpdate();
    }
}


SceneDevice* createSceneDoseMeter(Device* device)
{
    return new SceneDoseMeter(device);
}


DoseMeter::DoseMeter()
    : Device()
{
    on_ = false;
    color_ << 1.0, 1.0, 1.0;
    integralDose_ << 0.0, 0.0;
    doseRate_ = 0.0;
    hasShield_ = false;
    isShield_ = false;
    material_ .clear();
    thickness_ = 0.0;
}


const char* DoseMeter::typeName() const
{
    return "DoseMeter";
}


void DoseMeter::copyStateFrom(const DoseMeter& other)
{
    on_ = other.on_;
    color_ = other.color_;
    integralDose_ = other.integralDose_;
    doseRate_= other.doseRate_;
    hasShield_ = other.hasShield_;
    isShield_ = other.isShield_;
    material_ = other.material_;
    thickness_ = other.thickness_;
}


void DoseMeter::copyStateFrom(const DeviceState& other)
{
    if(typeid(other) != typeid(DoseMeter)) {
        throw std::invalid_argument("Type mismatch in the Device::copyStateFrom function");
    }
    copyStateFrom(static_cast<const DoseMeter&>(other));
}


DoseMeter::DoseMeter(const DoseMeter& org, bool copyStateOnly)
    : Device(org, copyStateOnly)
{
    copyStateFrom(org);

    if(!copyStateOnly) {

    }
}


void DoseMeter::setShield(const bool on)
{
    if(hasShield_) {
        isShield_ = on;
    }
}


DeviceState* DoseMeter::cloneState() const
{
    return new DoseMeter(*this, false);
}


Referenced* DoseMeter::doClone(CloneMap*) const
{
    return new DoseMeter(*this, false);
}


void DoseMeter::forEachActualType(std::function<bool(const std::type_info& type)> func)
{
    if(!func(typeid(DoseMeter))) {
       Device::forEachActualType(func);
    }
}


void DoseMeter::clearState()
{
    color_ << 1.0, 1.0, 1.0;
    doseRate_ = 0.0;
    integralDose_[1] = integralDose_[0];
}


int DoseMeter::stateSize() const
{
    return 10;
}


const double* DoseMeter::readState(const double* buf)
{
    int i = 0;
    on_ = buf[i++];
    color_[0] = buf[i++];
    color_[1] = buf[i++];
    color_[2] = buf[i++];
    integralDose_[0] = buf[i++];
    integralDose_[1] = buf[i++];
    doseRate_ = buf[i++];
    hasShield_ = buf[i++];
    isShield_ = buf[i++];
    thickness_ = buf[i++];
    return buf + i;
}


double* DoseMeter::writeState(double* out_buf) const
{
    int i = 0;
    out_buf[i++] = on_ ? 1.0 : 0.0;
    out_buf[i++] = color_[0];
    out_buf[i++] = color_[1];
    out_buf[i++] = color_[2];
    out_buf[i++] = integralDose_[0];
    out_buf[i++] = integralDose_[1];
    out_buf[i++] = doseRate_;
    out_buf[i++] = hasShield_;
    out_buf[i++] = isShield_;
    out_buf[i++] = thickness_;
    return out_buf + i;
}


bool DoseMeter::on() const
{
    return on_;
}


void DoseMeter::on(bool on)
{
    on_ = on;
}


bool DoseMeter::readSpecifications(const Mapping* info)
{
    info->read("on", on_);
    read(info, "color", color_);
    info->read("material", material_);
    info->read("thickness", thickness_);
    info->read({ "offset_dose", "offsetDose" }, integralDose_[0]);
    if(!material_.empty() && thickness_ > 0.0) {
        hasShield_ = true;
        isShield_ = true;
    }

    return true;
}


bool DoseMeter::writeSpecifications(Mapping* info) const
{
    info->write("on", on_);
    write(info, "color", color_);
    info->write("material", material_);
    info->write("thickness", thickness_);
    info->write("offset_dose", integralDose_[0]);

    return true;
}


namespace
{

bool readDoseMeter(StdBodyLoader* loader, const Mapping* info)
{
    DoseMeterPtr meter = new DoseMeter;
    if(!meter->readSpecifications(info)) {
       meter.reset();
    }
    bool result = false;
    if(meter) {
       result = loader->readDevice(meter, info);
    }
    return result;
}


struct DoseMeterRegistration
{
    DoseMeterRegistration() {
       StdBodyLoader::registerNodeType("DoseMeter", readDoseMeter);
       StdBodyWriter::registerDeviceWriter<DoseMeter>(
          "DoseMeter",
          [](StdBodyWriter* /* writer */, Mapping* info, const DoseMeter* meter) {
             return meter->writeSpecifications(info);
          });
       SceneDevice::registerSceneDeviceFactory<DoseMeter>(createSceneDoseMeter);
    }
} registrationDoseMeter;

}
