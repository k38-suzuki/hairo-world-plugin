/**
   @author Kenta Suzuki
*/

#ifndef CNOID_CFD_PLUGIN_WING_DEVICE_H
#define CNOID_CFD_PLUGIN_WING_DEVICE_H

#include <cnoid/Device>
#include <memory>
#include "exportdecl.h"

namespace cnoid {

class Mapping;

class CNOID_EXPORT WingDevice : public Device
{
public:
    WingDevice();
    WingDevice(const WingDevice& org, bool copyStateOnly = false);

    virtual const char* typeName() const override;
    void copyStateFrom(const WingDevice& other);
    virtual void copyStateFrom(const DeviceState& other) override;
    virtual DeviceState* cloneState() const override;
    virtual void forEachActualType(std::function<bool(const std::type_info& type)> func) override;
    virtual void clearState() override;
    virtual int stateSize() const override;
    virtual const double* readState(const double* buf) override;
    virtual double* writeState(double* out_buf) const override;

    bool on() const override;
    void on(bool on) override;

    double cl() const { return cl_; }
    void setCl(double cl) { cl_ = cl; }
    double wingspan() const { return wingspan_; }
    void setWingspan(double wingspan) { wingspan_ = wingspan; }
    double chordLength() const { return chordLength_; }
    void setChordLength(double chordLength) { chordLength_ = chordLength; } 
    void setSymbol(bool symbol) { symbol_ = symbol; }
    bool symbol() const { return symbol_; }   

    bool readSpecifications(const Mapping* info);
    bool writeSpecifications(Mapping* info) const;

protected:
    virtual Referenced* doClone(CloneMap* cloneMap) const override;

private:
    bool on_;
    double cl_;
    double wingspan_;
    double chordLength_;
    bool symbol_;

    struct Spec {

    };
    std::unique_ptr<Spec> spec;

    void copyWingDeviceStateFrom(const WingDevice& other);
};

typedef ref_ptr<WingDevice> WingDevicePtr;

}

#endif // CNOID_CFD_PLUGIN_WING_DEVICE_H
