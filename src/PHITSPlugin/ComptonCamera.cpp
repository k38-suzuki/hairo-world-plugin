/**
   @author Kenta Suzuki
*/

#include "ComptonCamera.h"
#include <cnoid/EigenArchive>
#include <cnoid/StdBodyFileUtil>
#include "GammaData.h"

using namespace std;
using namespace cnoid;


ComptonCamera::ComptonCamera()
{
    setDataType(GammaData::COMPTON);
    elementWidth_ = 0.5; // unit[cm] range 0.1 to 1.0
    scattererThickness_ = 0.5; // unit[cm] range 0.1 to 1.0
    absorberThickness_ = 0.5; // unit[cm] range 0.1 to 1.0
    distance_ = 5.5; // unit[cm] range 1.0 to 10.0
    arm_ = 6.0; // unit[deg] range 1 to 10
}


ComptonCamera::ComptonCamera(const ComptonCamera& org, bool copyStateOnly)
    : GammaCamera(org, copyStateOnly)
{
    if(!copyStateOnly) {

    }
    copyComptonCameraStateFrom(org, false, org.isImageStateClonable());
}


const char* ComptonCamera::typeName() const
{
    return "ComptonCamera";
}


void ComptonCamera::copyStateFrom(const DeviceState& other)
{
    if(typeid(other) != typeid(ComptonCamera)) {
        throw std::invalid_argument("Type mismatch in the Device::copyStateFrom function");
    }
    copyComptonCameraStateFrom(static_cast<const ComptonCamera&>(other), true, true);
}


void ComptonCamera::copyComptonCameraStateFrom(const ComptonCamera& other, bool doCopyCameraState, bool doCopyImage)
{
    if(doCopyCameraState) {
        Camera::copyCameraStateFrom(other, true, doCopyImage);
    }

    elementWidth_ = other.elementWidth_;
    scattererThickness_ = other.scattererThickness_;
    absorberThickness_ = other.absorberThickness_;
    distance_ = other.distance_;
    arm_ = other.arm_;
}


DeviceState* ComptonCamera::cloneState() const
{
    return new ComptonCamera(*this, true);
}


void ComptonCamera::forEachActualType(std::function<bool(const std::type_info& type)> func)
{
    if(!func(typeid(ComptonCamera))) {
        Camera::forEachActualType(func);
    }
}


void ComptonCamera::clearState()
{
    Camera::clearState();
}


Referenced* ComptonCamera::doClone(CloneMap*) const
{
    return new ComptonCamera(*this, false);
}


int ComptonCamera::stateSize() const
{
    return 5;
}


const double* ComptonCamera::readState(const double* buf)
{
    Camera::readState(buf);

    int i = 0;
    elementWidth_ = buf[i++];
    scattererThickness_ = buf[i++];
    absorberThickness_ = buf[i++];
    distance_ = buf[i++];
    arm_ = buf[i++];
    return buf + i + Camera::stateSize();
}


double* ComptonCamera::writeState(double* out_buf) const
{
    Camera::writeState(out_buf);

    int i = 0;
    out_buf[i++] = elementWidth_;
    out_buf[i++] = scattererThickness_;
    out_buf[i++] = absorberThickness_;
    out_buf[i++] = distance_;
    out_buf[i++] = arm_;
    return out_buf + i + Camera::stateSize();
}


bool ComptonCamera::readSpecifications(const Mapping* info)
{
    if(!GammaCamera::readSpecifications(info)) {
        return false;
    }

    info->read({ "element_width", "elementWidth" }, elementWidth_);
    info->read({ "scatterer_thickness", "scattererThickness" }, scattererThickness_);
    info->read({ "absorber_thickness", "absorberThickness" }, absorberThickness_);
    info->read("distance", distance_);
    info->read("arm", arm_);

    return true;
}


bool ComptonCamera::writeSpecifications(Mapping* info) const
{
    if(!GammaCamera::writeSpecifications(info)) {
        return false;
    }

    info->write("element_width", elementWidth_);
    info->write("scatterer_thickness", scattererThickness_);
    info->write("absorber_thickness", absorberThickness_);
    info->write("distance", distance_);
    info->write("arm", arm_);

    return true;
}


namespace {

bool readComptonCamera(StdBodyLoader* loader, const Mapping* info)
{
    ComptonCameraPtr camera = new ComptonCamera;
    if(!camera->readSpecifications(info)) {
        camera.reset();
    }
    bool result = false;
    if(camera) {
        result = loader->readDevice(camera, info);
    }
    return result;
}


struct ComptonCameraRegistration
{
    ComptonCameraRegistration() {
        StdBodyLoader::registerNodeType("ComptonCamera", readComptonCamera);
        StdBodyWriter::registerDeviceWriter<ComptonCamera>(
            "ComptonCamera",
            [](StdBodyWriter* /* writer */, Mapping* info, const ComptonCamera* camera) {
                return camera->writeSpecifications(info);
            });
    }
} registrationComptonCamera;

}
