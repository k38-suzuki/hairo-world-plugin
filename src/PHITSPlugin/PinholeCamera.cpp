/**
   @author Kenta Suzuki
*/

#include "PinholeCamera.h"
#include <cnoid/EigenArchive>
#include <cnoid/StdBodyFileUtil>
#include "GammaData.h"

using namespace std;
using namespace cnoid;


PinholeCamera::PinholeCamera()
{
    setDataType(GammaData::PINHOLE);
    thickness_ = 2.0; // unit[cm] range 0.1 to 10.0
    pinholeOpening_ = 0.5; // unit[cm] range 0.01 to 0.5
}


PinholeCamera::PinholeCamera(const PinholeCamera& org, bool copyStateOnly)
    : GammaCamera(org, copyStateOnly)
{
    if(!copyStateOnly) {

    }
    copyPinholeCameraStateFrom(org, false, org.isImageStateClonable());
}


const char* PinholeCamera::typeName() const
{
    return "PinholeCamera";
}


void PinholeCamera::copyStateFrom(const DeviceState& other)
{
    if(typeid(other) != typeid(PinholeCamera)) {
        throw std::invalid_argument("Type mismatch in the Device::copyStateFrom function");
    }
    copyPinholeCameraStateFrom(static_cast<const PinholeCamera&>(other), true, true);
}


void PinholeCamera::copyPinholeCameraStateFrom(const PinholeCamera& other, bool doCopyCameraState, bool doCopyImage)
{
    if(doCopyCameraState) {
        Camera::copyCameraStateFrom(other, true, doCopyImage);
    }

    thickness_ = other.thickness_;
    pinholeOpening_ = other.pinholeOpening_;
}


DeviceState* PinholeCamera::cloneState() const
{
    return new PinholeCamera(*this, true);
}


void PinholeCamera::forEachActualType(std::function<bool(const std::type_info& type)> func)
{
    if(!func(typeid(PinholeCamera))) {
        Camera::forEachActualType(func);
    }
}


void PinholeCamera::clearState()
{
    Camera::clearState();
}


Referenced* PinholeCamera::doClone(CloneMap*) const
{
    return new PinholeCamera(*this, false);
}


int PinholeCamera::stateSize() const
{
    return 2;
}


const double* PinholeCamera::readState(const double* buf)
{
    Camera::readState(buf);

    int i = 0;
    thickness_ = buf[i++];
    pinholeOpening_ = buf[i++];
    return buf + i + Camera::stateSize();
}


double* PinholeCamera::writeState(double* out_buf) const
{
    Camera::writeState(out_buf);

    int i = 0;
    out_buf[i++] = thickness_;
    out_buf[i++] = pinholeOpening_;
    return out_buf + i + Camera::stateSize();
}


bool PinholeCamera::readSpecifications(const Mapping* info)
{
    if(!GammaCamera::readSpecifications(info)) {
        return false;
    }

    info->read("thickness", thickness_);
    info->read({ "pinhole_opening", "pinholeOpening" }, pinholeOpening_);

    return true;
}


bool PinholeCamera::writeSpecifications(Mapping* info) const
{
    if(!GammaCamera::writeSpecifications(info)) {
        return false;
    }

    info->write("thickness", thickness_);
    info->write("pinhole_opening", pinholeOpening_);

    return true;
}


namespace {

bool readPinholeCamera(StdBodyLoader* loader, const Mapping* info)
{
    PinholeCameraPtr camera = new PinholeCamera;
    if(!camera->readSpecifications(info)) {
        camera.reset();
    }
    bool result = false;
    if(camera) {
        result = loader->readDevice(camera, info);
    }
    return result;
}


struct PinholeCameraRegistration
{
    PinholeCameraRegistration() {
        StdBodyLoader::registerNodeType("PinholeCamera", readPinholeCamera);
        StdBodyLoader::registerNodeType("GammaCamera", readPinholeCamera);
        StdBodyWriter::registerDeviceWriter<PinholeCamera>(
            "PinholeCamera",
            [](StdBodyWriter* /* writer */, Mapping* info, const PinholeCamera* camera) {
                return camera->writeSpecifications(info);
            });
    }
} registrationPinholeCamera;

}
