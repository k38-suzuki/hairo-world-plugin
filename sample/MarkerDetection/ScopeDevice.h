/**
   @author Kenta Suzuki
*/

#ifndef CNOID_MARKER_DETECT_PLUGIN_SCOPE_DEVICE_H
#define CNOID_MARKER_DETECT_PLUGIN_SCOPE_DEVICE_H

#include <cnoid/Device>
#include <cnoid/ValueTree>
#include "exportdecl.h"

namespace cnoid {

class CNOID_EXPORT ScopeDevice : public Device
{
public:
    ScopeDevice();
    ScopeDevice(const ScopeDevice& org, bool copyStateOnly = false);
    virtual const char* typeName() const override;
    void copyStateFrom(const ScopeDevice& other);
    virtual void copyStateFrom(const DeviceState& other) override;
    virtual DeviceState* cloneState() const override;
    virtual void forEachActualType(std::function<bool(const std::type_info& type)> func) override;
    virtual void on(const bool on) override;
    virtual bool on() const override;
    virtual int stateSize() const override;
    virtual const double* readState(const double* buf) override;
    virtual double* writeState(double* out_buf) const override;

    bool readSpecifications(const Mapping* info);
    bool writeSpecifications(Mapping* info) const;

    void setFieldOfView(const int& fieldOfView) { fieldOfView_ = fieldOfView; }
    int fieldOfView() const { return fieldOfView_; }
    void setFocalLength(const double& focalLength) { focalLength_ = focalLength; }
    double focalLength() const { return focalLength_; }
    void setAspectRatio(const Vector2& aspectRatio) { aspectRatio_ = aspectRatio; }
    Vector2 aspectRatio() const { return aspectRatio_; }
    void setDiffuseColor(const Vector3& diffuseColor) { diffuseColor_ = diffuseColor; }
    Vector3 diffuseColor() const { return diffuseColor_; }
    void setAmbientIntensity(const float& ambientIntensity) { ambientIntensity_ = ambientIntensity; }
    float ambientIntensity() const { return ambientIntensity_; }
    void setShininess(const float& shininess) { shininess_ = shininess; }
    float shininess() const { return shininess_; }
    void setTransparency(const float& transparency) { transparency_ = transparency; }
    float transparency() const { return transparency_; }

protected:
    virtual Referenced* doClone(CloneMap* cloneMap) const override;

private:
    bool on_;
    int fieldOfView_;
    double focalLength_;
    Vector2 aspectRatio_;
    Vector3 diffuseColor_;
    float ambientIntensity_;
    float shininess_;
    float transparency_;
};

typedef ref_ptr<ScopeDevice> ScopeDevicePtr;

}

#endif
