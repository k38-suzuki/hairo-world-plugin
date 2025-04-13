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
      VisualEffect()
{
}

NoisyCamera::NoisyCamera(const NoisyCamera& org, bool copyStateOnly)
    : Camera(org, copyStateOnly),
      VisualEffect(org)
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
    return Camera::stateSize() + 15;
}

const double* NoisyCamera::readState(const double* buf, int size)
{
    buf = Camera::readState(buf, size);
    setHsv(Eigen::Map<const Vector3>(buf));
    setRgb(Eigen::Map<const Vector3>(buf));
    setCoefB(buf[6]);
    setCoefD(buf[7]);
    setStdDev(buf[8]);
    setSaltAmount(buf[9]);
    setSaltChance(buf[10]);
    setPepperAmount(buf[11]);
    setPepperChance(buf[12]);
    setMosaicChance(buf[13]);
    setKernel(buf[14]);
    return buf + 15;
}

double* NoisyCamera::writeState(double* out_buf) const
{
    out_buf = Camera::writeState(out_buf);
    Eigen::Map<Vector3>(out_buf) << hsv();
    Eigen::Map<Vector3>(out_buf) << rgb();
    out_buf[6] = coefB();
    out_buf[7] = coefD();
    out_buf[8] = stdDev();
    out_buf[9] = saltAmount();
    out_buf[10] = saltChance();
    out_buf[11] = pepperAmount();
    out_buf[12] = pepperChance();
    out_buf[13] = mosaicChance();
    out_buf[14] = kernel();
    return out_buf + 15;
}

bool NoisyCamera::readSpecifications(const Mapping* info)
{
    if(!Camera::readSpecifications(info)) {
        return false;
    }

    this->readCameraInfo(info);

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
    info->write("salt_amount", saltAmount());
    info->write("salt_chance", saltChance());
    info->write("pepper_amount", pepperAmount());
    info->write("pepper_chance", pepperChance());
    info->write("mosaic_chance", mosaicChance());
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
    setSaltAmount(other.saltAmount());
    setSaltChance(other.saltChance());
    setPepperAmount(other.pepperAmount());
    setPepperChance(other.pepperChance());
    setMosaicChance(other.mosaicChance());
    setKernel(other.kernel());
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

struct NoisyCameraRegistration {
    NoisyCameraRegistration()
    {
        StdBodyLoader::registerNodeType("NoisyCamera", readNoisyCamera);
        StdBodyWriter::registerDeviceWriter<NoisyCamera>(
            "NoisyCamera", [](StdBodyWriter* /* writer */, Mapping* info, const NoisyCamera* camera) {
                return camera->writeSpecifications(info);
            });
    }
} registrationNoisyCamera;

} // namespace