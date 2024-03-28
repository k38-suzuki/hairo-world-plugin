/**
   @author Kenta Suzuki
*/

#include "NoisyCamera.h"
#include <cnoid/EigenArchive>
#include <cnoid/StdBodyFileUtil>
#include <cnoid/ValueTree>

using namespace std;
using namespace cnoid;

NoisyCamera::NoisyCamera()
    : spec(new Spec),
      Camera(),
      VFXEffects()
{

}


NoisyCamera::NoisyCamera(const NoisyCamera& org, bool copyStateOnly)
    : Camera(org, copyStateOnly),
      VFXEffects(org)
{
    if(!copyStateOnly) {
        spec = make_unique<Spec>();
        if(org.spec) {

        } else {

        }
    }
    copyNoisyCameraStateFrom(org, false, org.isImageStateClonable());
}


const char* NoisyCamera::typeName() const
{
    return "NoisyCamera";
}


void NoisyCamera::copyStateFrom(const DeviceState& other)
{
    if(typeid(other) != typeid(NoisyCamera)) {
        throw std::invalid_argument("Type mismatch in the Device::copyStateFrom function");
    }
    copyNoisyCameraStateFrom(static_cast<const NoisyCamera&>(other), true, true);
}


DeviceState* NoisyCamera::cloneState() const
{
    return new NoisyCamera(*this, true);
}


void NoisyCamera::forEachActualType(std::function<bool(const std::type_info& type)> func)
{
    if(!func(typeid(NoisyCamera))) {
        Camera::forEachActualType(func);
    }
}


void NoisyCamera::clearState()
{
    Camera::clearState();
}


int NoisyCamera::stateSize() const
{
    return Camera::stateSize() + 13;
}


const double* NoisyCamera::readState(const double* buf)
{
    buf = VisionSensor::readState(buf);
    setHsv(Eigen::Map<const Vector3>(buf));
    setRgb(Eigen::Map<const Vector3>(buf));
    setCoefB(buf[6]);
    setCoefD(buf[7]);
    setStdDev(buf[8]);
    setSalt(buf[9]);
    setPepper(buf[10]);
    setMosaic(buf[11]);
    setKernel(buf[12]);
    return buf + 13;
}


double* NoisyCamera::writeState(double* out_buf) const
{
    out_buf = VisionSensor::writeState(out_buf);
    Eigen::Map<Vector3>(out_buf) << hsv();
    Eigen::Map<Vector3>(out_buf) << rgb();
    out_buf[6] = coefB();
    out_buf[7] = coefD();
    out_buf[8] = stdDev();
    out_buf[9] = salt();
    out_buf[10] = pepper();
    out_buf[11] = mosaic();
    out_buf[12] = kernel();
    return out_buf + 13;
}


bool NoisyCamera::readSpecifications(const Mapping* info)
{
    if(!Camera::readSpecifications(info)) {
        return false;
    }

    Vector3 v;
    if(read(info, "hsv", v)) {
        setHsv(v);
    }
    if(read(info, "rgb", v)) {
        setRgb(v);
    }
    setCoefB(info->get({ "coef_b", "coefB" }, 0.0));
    setCoefD(info->get({ "coef_d", "coefD" }, 1.0));
    setStdDev(info->get({ "std_dev", "stdDev" }, 0.0));
    setSalt(info->get("salt", 0.0));
    setPepper(info->get("pepper", 0.0));
    setMosaic(info->get("mosaic", 0.0));
    setKernel(info->get("kernel", 16));

    return true;
}


bool NoisyCamera::writeSpecifications(Mapping* info) const
{
    if(!Camera::writeSpecifications(info)) {
        return false;
    }

    write(info, "hsv", Vector3(hsv()));
    write(info, "rgb", Vector3(rgb()));
    info->write("coef_b", coefB());
    info->write("coef_d", coefD());
    info->write("std_dev", stdDev());
    info->write("salt", salt());
    info->write("pepper", pepper());
    info->write("mosaic", mosaic());
    info->write("kernel", kernel());

    return true;
}


void NoisyCamera::copyNoisyCameraStateFrom(const NoisyCamera& other, bool doCopyCameraState, bool doCopyImage)
{
    if(doCopyCameraState) {
        Camera::copyCameraStateFrom(other, true, doCopyImage);
    }

    setHsv(other.hsv());
    setRgb(other.rgb());
    setCoefB(other.coefB());
    setCoefD(other.coefD());
    setStdDev(other.stdDev());
    setSalt(other.salt());
    setPepper(other.pepper());
}


Referenced* NoisyCamera::doClone(CloneMap* cloneMap) const
{
    return new NoisyCamera(*this, false);
}


namespace {

// StdBodyFileDeviceTypeRegistration<NoisyCamera>
// registerHolderDevice(
//     "Camera",
//     nullptr,
//     [](StdBodyWriter* writer, Mapping* info, const NoisyCamera* camera)
//     {
//         return camera->writeSpecifications(info);
//     });

bool readNoisyCamera(StdBodyLoader* loader, const Mapping* info)
{
    NoisyCameraPtr camera = new NoisyCamera;
    if(!camera->readSpecifications(info)) {
        camera.reset();
    }

    bool result = false;
    if(camera) {
        result = loader->readDevice(camera, info);
    }
    return result;
}


struct NoisyCameraRegistration
{
    NoisyCameraRegistration() {
        StdBodyLoader::registerNodeType("NoisyCamera", readNoisyCamera);
        StdBodyWriter::registerDeviceWriter<NoisyCamera>(
                    "NoisyCamera",
                    [](StdBodyWriter* /* writer */, Mapping* info, const NoisyCamera* camera) {
            return camera->writeSpecifications(info);
        });
    }
} registrationNoisyCamera;

}
