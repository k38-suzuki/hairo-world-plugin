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
      CameraEffect()
{
    setImageType(NO_IMAGE);
    generator = new ImageGenerator;
}


NoisyCamera::NoisyCamera(const NoisyCamera& org, bool copyStateOnly)
    : Camera(org, copyStateOnly),
      CameraEffect()
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


const Image& NoisyCamera::constImage() const
{
    double hue = CameraEffect::hsv()[0];
    double saturation = CameraEffect::hsv()[1];
    double value = CameraEffect::hsv()[2];
    double red = CameraEffect::rgb()[0];
    double green = CameraEffect::rgb()[1];
    double blue = CameraEffect::rgb()[2];
    double coefB = CameraEffect::coefB();
    double coefD = CameraEffect::coefD();
    double stdDev = CameraEffect::stdDev();
    double salt = CameraEffect::salt();
    double pepper = CameraEffect::pepper();
    bool flipped = CameraEffect::flip();
    FilterType filterType = CameraEffect::filterType();

    Image image = *Camera::sharedImage();
    if(hue > 0.0 || saturation > 0.0 || value > 0.0) {
        generator->hsv(image, hue, saturation, value);
    }
    if(red > 0.0 || green > 0.0 || blue > 0.0) {
        generator->rgb(image, red, green, blue);
    }
    if(flipped) {
        generator->flippedImage(image);
    }

    if(stdDev > 0.0) {
        generator->gaussianNoise(image, stdDev);
    }
    if(salt > 0.0 || pepper > 0.0) {
        generator->saltPepperNoise(image, salt, pepper);
    }
    if(filterType == GAUSSIAN_3X3) {
        generator->gaussianFilter(image, 3);
    } else if(filterType == GAUSSIAN_5X5) {
        generator->gaussianFilter(image, 5);
    } else if(filterType == SOBEL) {
        generator->sobelFilter(image);
    } else if(filterType == PREWITT) {
        generator->prewittFilter(image);
    }
    if(coefB < 0.0 || coefD > 1.0) {
        generator->barrelDistortion(image, coefB, coefD);
    }

    std::shared_ptr<Image> sharedImage = std::make_shared<Image>(image);
    return *sharedImage;
}


int NoisyCamera::stateSize() const
{
    return Camera::stateSize() + 12;
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
    setFlip(buf[11]);
    return buf + 12;
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
    out_buf[11] = flip() ? 1.0 : 0.0;
    return out_buf + 12;
}


bool NoisyCamera::readSpecifications(const Mapping* info)
{
    if(!Camera::readSpecifications(info)) {
        return false;
    }

    Vector3 v;
    read(info, "hsv", v);
    setHsv(v);
    read(info, "rgb", v);
    setRgb(v);

    double d;
    info->read({ "coef_b", "coefB" }, d);
    setCoefB(d);
    info->read({ "coef_d", "coefD" }, d);
    setCoefD(d);
    info->read({ "std_dev", "stdDev" }, d);
    setStdDev(d);
    info->read("salt", d);
    setSalt(d);
    info->read("pepper", d);
    setPepper(d);

    bool b;
    info->read("flip", b);
    setFlip(b);

    string symbol;
    if(info->read({ "filter_type", "filterType" }, symbol)) {
        if(symbol == "NO_FILTER") {
            setFilterType(NO_FILTER);
        } else if(symbol == "GAUSSIAN_3X3") {
            setFilterType(GAUSSIAN_3X3);
        } else if(symbol == "GAUSSIAN_5X5") {
            setFilterType(GAUSSIAN_5X5);
        } else if(symbol == "SOBEL") {
            setFilterType(SOBEL);
        } else if(symbol == "PREWITT") {
            setFilterType(PREWITT);
        }
    }

    return true;
}


bool NoisyCamera::writeSpecifications(Mapping* info) const
{
    if(!Camera::writeSpecifications(info)) {
        return false;
    }

    write(info, "hsv", hsv());
    write(info, "rgb", rgb());
    info->write("coef_b", coefB());
    info->write("coef_d", coefD());
    info->write("std_dev", stdDev());
    info->write("salt", salt());
    info->write("pepper", pepper());
    info->write("flip", flip());
    if(filterType() == NO_FILTER) {
        info->write("filter_type", "NO_FILTER");
    } else if(filterType() == GAUSSIAN_3X3) {
        info->write("filter_type", "GAUSSIAN_3X3");
    } else if(filterType() == GAUSSIAN_5X5) {
        info->write("filter_type", "GAUSSIAN_5X5");
    } else if(filterType() == SOBEL) {
        info->write("filter_type", "SOBEL");
    } else if(filterType() == PREWITT) {
        info->write("filter_type", "PREWITT");
    }

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
    setFlip(other.flip());
    setFilterType(other.filterType());
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