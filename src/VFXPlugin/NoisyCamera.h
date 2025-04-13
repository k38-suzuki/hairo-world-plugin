/**
   @author Kenta Suzuki
*/

#ifndef CNOID_VFX_PLUGIN_NOISY_CAMERA_H
#define CNOID_VFX_PLUGIN_NOISY_CAMERA_H

#include <cnoid/Camera>
#include <cnoid/CustomEffect>
#include "exportdecl.h"

namespace cnoid {

class CNOID_EXPORT NoisyCamera : public Camera, public VisualEffect
{
public:
    NoisyCamera();
    NoisyCamera(const NoisyCamera& org, bool copyStateOnly = false);

    virtual const char* typeName() const override;
    virtual void copyStateFrom(const DeviceState& other) override;
    virtual DeviceState* cloneState() const override;
    virtual void forEachActualType(std::function<bool(const std::type_info& type)> func) override;
    virtual void clearState() override;

    virtual int stateSize() const override;
    virtual const double* readState(const double* buf, int size) override;
    virtual double* writeState(double* out_buf) const override;

    bool readSpecifications(const Mapping* info);
    bool writeSpecifications(Mapping* info) const;

protected:
    void copyNoisyCameraStateFrom(const NoisyCamera& other, bool doCopyCameraState, bool doCopyImage);
    virtual Referenced* doClone(CloneMap* cloneMap) const override;

private:
    struct Spec {
    };

    std::unique_ptr<Spec> spec;
};

typedef ref_ptr<NoisyCamera> NoisyCameraPtr;

} // namespace cnoid

#endif // CNOID_VFX_PLUGIN_NOISY_CAMERA_H