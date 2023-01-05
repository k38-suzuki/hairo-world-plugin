/**
   \file
   \author Kenta Suzuki
*/

#ifndef CNOID_PHITS_PLUGIN_PINHOLE_CAMERA_H
#define CNOID_PHITS_PLUGIN_PINHOLE_CAMERA_H

#include "GammaCamera.h"
#include "exportdecl.h"

namespace cnoid {

class CNOID_EXPORT PinholeCamera: public GammaCamera
{
public:
    PinholeCamera();
    PinholeCamera(const PinholeCamera& org, bool copyStateOnly = false);
    virtual const char* typeName() const override;
    virtual void copyStateFrom(const DeviceState& other) override;
    virtual DeviceState* cloneState() const override;
    virtual void forEachActualType(std::function<bool(const std::type_info& type)> func) override;
    virtual void clearState() override;
    virtual int stateSize() const override;
    virtual const double* readState(const double* buf) override;
    virtual double* writeState(double* out_buf) const override;

    void setMaterialThickness(double d) { thickness_ = std::max(0.1, std::min(10.0, d)); }
    double thickness() const { return thickness_; }
    void setPinholeOpening(double d) { pinholeOpening_ = std::max(0.01, std::min(0.5, d)); }
    double pinholeOpening() const { return pinholeOpening_; }

    bool readSpecifications(const Mapping* info);
    bool writeSpecifications(Mapping* info) const;

protected:
    void copyPinholeCameraStateFrom(const PinholeCamera& other, bool doCopyCameraState, bool doCopyImage);
    virtual Referenced* doClone(CloneMap* cloneMap) const override;

private:
    double thickness_;
    double pinholeOpening_;
};

typedef ref_ptr<PinholeCamera> PinholeCameraPtr;

}

#endif // CNOID_PHITS_PLUGIN_PINHOLE_CAMERA_H