/**
   @author Kenta Suzuki
*/

#include "GammaCamera.h"
#include <cnoid/EigenArchive>
#include <cnoid/StdBodyFileUtil>

using namespace std;
using namespace cnoid;


GammaCamera::GammaCamera()
{
    isReady_ = false;
    dataType_ = 0;
    resolution_ << 8, 8;
    material_.clear();
}


GammaCamera::GammaCamera(const GammaCamera& org, bool copyStateOnly)
    : Camera(org, copyStateOnly)
{
    if(!copyStateOnly) {

    }
    copyGammaCameraStateFrom(org, false, org.isImageStateClonable());
}


const char* GammaCamera::typeName() const
{
    return "GammaCamera";
}


void GammaCamera::copyStateFrom(const DeviceState& other)
{
    if(typeid(other) != typeid(GammaCamera)) {
        throw std::invalid_argument("Type mismatch in the Device::copyStateFrom function");
    }
    copyGammaCameraStateFrom(static_cast<const GammaCamera&>(other), true, true);
}


void GammaCamera::copyGammaCameraStateFrom(const GammaCamera& other, bool doCopyCameraState, bool doCopyImage)
{
    if(doCopyCameraState) {
        Camera::copyCameraStateFrom(other, true, doCopyImage);
    }

    isReady_ = other.isReady_;
    dataType_ = other.dataType_;
    resolution_ = other.resolution_;
    material_ = other.material_;
}


Referenced* GammaCamera::doClone(CloneMap*) const
{
    return new GammaCamera(*this, false);
}


DeviceState* GammaCamera::cloneState() const
{
    return new GammaCamera(*this, true);
}


void GammaCamera::forEachActualType(std::function<bool(const std::type_info& type)> func)
{
    if(!func(typeid(GammaCamera))) {
        Camera::forEachActualType(func);
    }
}


void GammaCamera::clearState()
{
    Camera::clearState();
}


bool GammaCamera::readSpecifications(const Mapping* info)
{
    if(!Camera::readSpecifications(info)) {
        return false;
    }

    read(info, "resolution", resolution_);
    info->read("material", material_);

    return true;
}


bool GammaCamera::writeSpecifications(Mapping* info) const
{
    if(!Camera::writeSpecifications(info)) {
        return false;
    }

    write(info, "resolution", resolution_);
    info->write("material", material_);

    return true;
}


namespace {

StdBodyFileDeviceTypeRegistration<GammaCamera>
registerHolderDevice(
    "GammaCamera",
    nullptr,
    [](StdBodyWriter* writer, Mapping* info, const GammaCamera* camera)
    {
        return camera->writeSpecifications(info);
    });

}
