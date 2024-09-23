/**
   @author Kenta Suzuki
*/

#ifndef CNOID_PHITS_PLUGIN_GAMMA_CAMERA_H
#define CNOID_PHITS_PLUGIN_GAMMA_CAMERA_H

#include <cnoid/Camera>
#include "GammaData.h"
#include "exportdecl.h"

namespace cnoid {

class CNOID_EXPORT GammaCamera : public Camera
{
public:
    GammaCamera();
    GammaCamera(const GammaCamera& org, bool copyStateOnly = false);
    virtual const char* typeName() const override;
    virtual void copyStateFrom(const DeviceState& other) override;
    virtual DeviceState* cloneState() const override;
    virtual void forEachActualType(std::function<bool(const std::type_info& type)> func) override;
    virtual void clearState() override;

    GammaData& gammaData() { return gammaData_; }

    void setReady(bool isReady) { isReady_ = isReady; }
    bool isReady() const { return isReady_; }
    void setDataType(const int& dataType) { dataType_ = dataType; }
    int dataType() const { return dataType_; }
    void setResolution(Vector2 resolution) { resolution_ = resolution; }
    Vector2 resolution() const { return resolution_; }
    void setMaterial(const std::string& material) { material_ = material; }
    std::string material() const { return material_; }

    bool readSpecifications(const Mapping* info);
    bool writeSpecifications(Mapping* info) const;

protected:
    void copyGammaCameraStateFrom(const GammaCamera& other, bool doCopyCameraState, bool doCopyImage);
    virtual Referenced* doClone(CloneMap* cloneMap) const override;

private:
    GammaData gammaData_;
    bool isReady_;
    int dataType_;
    Vector2 resolution_;
    std::string material_;

};

typedef ref_ptr<GammaCamera> GammaCameraPtr;

}

#endif // CNOID_PHITS_PLUGIN_GAMMA_CAMERA_H
