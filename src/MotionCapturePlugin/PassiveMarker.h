/**
   \file
   \author Kenta Suzuki
*/

#ifndef CNOID_MOTIONCAPTUREPLUGIN_PASSIVEMARKER_H
#define CNOID_MOTIONCAPTUREPLUGIN_PASSIVEMARKER_H

#include <cnoid/Device>
#include <cnoid/SceneMarkers>
#include <cnoid/ValueTree>
#include <memory>
#include "exportdecl.h"

namespace cnoid {

class CNOID_EXPORT PassiveMarker : public Device
{
public:
    PassiveMarker();
    PassiveMarker(const PassiveMarker& org, bool copyStateOnly = false);
    virtual const char* typeName() const override;
    void copyStateFrom(const PassiveMarker& other);
    virtual void copyStateFrom(const DeviceState& other) override;
    virtual DeviceState* cloneState() const override;
    virtual void forEachActualType(std::function<bool(const std::type_info& type)> func) override;
    virtual void clearState() override;
    virtual bool on() const override;
    virtual void on(bool on) override;
    virtual int stateSize() const override;
    virtual const double* readState(const double* buf) override;
    virtual double* writeState(double* out_buf) const override;

    bool readSpecifications(const Mapping* info);
    bool writeSpecifications(Mapping* info) const;

    void setRadius(const double& radius) { radius_ = radius; }
    double radius() const { return radius_; }
    void setColor(const Vector3& color) { color_ = color; }
    Vector3 color() const { return color_; }
    void setTransparency(const double& transparency) { transparency_ = transparency; }
    double transparency() const { return transparency_; }
    void setSymbol(const bool& symbol) { symbol_ = symbol; }
    bool symbol() const { return symbol_; }

protected:
    virtual Referenced* doClone(CloneMap* cloneMap) const override;

private:
    bool on_;
    double radius_;
    Vector3 color_;
    double transparency_;
    bool symbol_;
};

typedef ref_ptr<PassiveMarker> PassiveMarkerPtr;

}

#endif // CNOID_MOTIONCAPTUREPLUGIN_PASSIVEMARKER_H
