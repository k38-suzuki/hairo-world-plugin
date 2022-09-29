/**
   \file
   \author Kenta Suzuki
*/

#ifndef CNOID_PHITSPLUGIN_COMPTONCAMERA_H
#define CNOID_PHITSPLUGIN_COMPTONCAMERA_H

#include "GammaCamera.h"
#include "exportdecl.h"

namespace cnoid {

class CNOID_EXPORT ComptonCamera: public GammaCamera
{
public:
    ComptonCamera();
    ComptonCamera(const ComptonCamera& org, bool copyStateOnly = false);
    virtual const char* typeName() const override;
    virtual void copyStateFrom(const DeviceState& other) override;
    virtual DeviceState* cloneState() const override;
    virtual void forEachActualType(std::function<bool(const std::type_info& type)> func) override;
    virtual void clearState() override;
    virtual int stateSize() const override;
    virtual const double* readState(const double* buf) override;
    virtual double* writeState(double* out_buf) const override;

    void setElementWidth(double d) { elementWidth_ = std::max(0.1, std::min(1.0, d)); }
    double elementWidth() const { return elementWidth_; }
    void setScattererThickness(double d) { scattererThickness_ = std::max(0.1, std::min(1.0, d)); }
    double scattererThickness() const { return scattererThickness_; }
    void setAbsorberThickness(double d) { absorberThickness_ = std::max(0.1, std::min(1.0, d)); }
    double absorberThickness() const { return absorberThickness_; }
    void setDistance(double d) { distance_ = std::max(1.0, std::min(10.0, d)); }
    double distance() const { return distance_; }
    void setArm(double d) { arm_ = std::max(1.0, std::min(10.0, d)); }
    double arm() const { return arm_; }

    bool readSpecifications(const Mapping* info);
    bool writeSpecifications(Mapping* info) const;

protected:
    void copyComptonCameraStateFrom(const ComptonCamera& other, bool doCopyCameraState, bool doCopyImage);
    virtual Referenced* doClone(CloneMap* cloneMap) const override;

private:
    double elementWidth_;
    double scattererThickness_;
    double absorberThickness_;
    double distance_;
    double arm_;
};

typedef ref_ptr<ComptonCamera> ComptonCameraPtr;

}

#endif // CNOID_PHITSPLUGIN_COMPTONCAMERA_H
