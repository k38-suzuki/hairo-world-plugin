/**
   @author Kenta Suzuki
*/

#include "MultiColliderItem.h"
#include <cnoid/Archive>
#include <cnoid/EigenArchive>
#include <cnoid/EigenUtil>
#include <cnoid/PutPropertyFunction>
#include <fmt/format.h>
#include "gettext.h"

using namespace std;
using namespace fmt;
using namespace cnoid;

MultiColliderItem::MultiColliderItem()
    : SimpleColliderItem(),
      CFDEffects(),
      TCEffects(),
      VFXEffects()
{
    colliderTypeSelection.setSymbol(CFD, N_("CFD"));
    colliderTypeSelection.setSymbol(TC, N_("TC"));
    colliderTypeSelection.setSymbol(VFX, N_("VFX"));
    colliderTypeSelection.select(CFD);
}


MultiColliderItem::MultiColliderItem(const MultiColliderItem& org)
    : SimpleColliderItem(org),
      CFDEffects(org),
      TCEffects(org),
      VFXEffects(org)
{
    colliderTypeSelection = org.colliderTypeSelection;
    
    switch(colliderTypeSelection.which()) {
    case CFD:
        setDiffuseColor(Vector3(0.0, 1.0, 1.0));
        break;
    case TC:
        setDiffuseColor(Vector3(1.0, 1.0, 0.0));
        break;

    case VFX:
        setDiffuseColor(Vector3(1.0, 0.0, 1.0));
        break;
    default:
        break;
    }
}


bool MultiColliderItem::setColliderType(int colliderId)
{
    if(!colliderTypeSelection.select(colliderId)) {
        return false;
    }
    notifyUpdate();
    return true;
}


Item* MultiColliderItem::doCloneItem(CloneMap* cloneMap) const
{
    return new MultiColliderItem(*this);
}


void MultiColliderItem::doPutProperties(PutPropertyFunction& putProperty)
{
    SimpleColliderItem::doPutProperties(putProperty);
    putProperty(_("Collider type"), colliderTypeSelection,
                [this](int which){ return setColliderType(which); });

    int colliderId = colliderType();
    switch(colliderId) {
    case CFD:
        putProperty.min(0.0).max(9999.0)(_("density"), density(),
                    [this](double value){
                        setDensity(value);
                        return true;
                    });

        putProperty.min(0.0).max(9999.0)(_("viscosity"), viscosity(),
                    [this](double value){
                        setViscosity(value);
                        return true;
                    });

        putProperty(_("steady flow"), format("{0:.3g} {1:.3g} {2:.3g}", steadyFlow().x(), steadyFlow().y(), steadyFlow().z()),
                    [this](const string& text){
                        Vector3 f;
                        if(toVector3(text, f)) {
                            setSteadyFlow(f);
                            return true;
                        }
                        return false;
                    });
        break;
    case TC:
        putProperty.min(0.0).max(100000.0)(_("inbound delay"), inboundDelay(),
                    [this](double value){
                        setInboundDelay(value);
                        return true;
                    });

        putProperty.min(0.0).max(11000000.0)(_("inbound rate"), inboundRate(),
                    [this](double value){
                        setInboundRate(value);
                        return true;
                    });

        putProperty.min(0.0).max(100.0)(_("inbound loss"), inboundLoss(),
                    [this](double value){
                        setInboundLoss(value);
                        return true;
                    });

        putProperty.min(0.0).max(100000.0)(_("outbound delay"), outboundDelay(),
                    [this](double value){
                        setOutboundDelay(value);
                        return true;
                    });

        putProperty.min(0.0).max(11000000.0)(_("outbound rate"), outboundRate(),
                    [this](double value){
                        setOutboundRate(value);
                        return true;
                    });

        putProperty.min(0.0).max(100.0)(_("outbound loss"), outboundLoss(),
                    [this](double value){
                        setOutboundLoss(value);
                        return true;
                    });

        putProperty(_("source IP"), format("{0}", source()),
                    [this](const string& text){
                        setSource(text);
                        return true;
                    });

        putProperty(_("destination IP"), format("{0}", destination()),
                    [this](const string& text){
                        setDestination(text);
                        return true;
                    });
        break;
    case VFX:
        putProperty(_("HSV"), format("{0:.3g} {1:.3g} {2:.3g}", hsv().x(), hsv().y(), hsv().z()),
                    [this](const string& text){
                        Vector3 c;
                        if(toVector3(text, c)) {
                            setHsv(c);
                            return true;
                        }
                        return false;
                    });

        putProperty(_("RGB"),  format("{0:.3g} {1:.3g} {2:.3g}", rgb().x(), rgb().y(), rgb().z()),
                    [this](const string& text){
                        Vector3 c;
                        if(toVector3(text, c)) {
                            setRgb(c);
                            return true;                        
                        }
                        return false;
                    });

        putProperty.min(-1.0).max(0.0)(_("coef B"), coefB(),
                    [this](double value){
                        setCoefB(value);
                        return true;
                    });

        putProperty.min(1.0).max(32.0)(_("coef D"), coefD(),
                    [this](double value){
                        setCoefD(value);
                        return true;
                    });

        putProperty.min(0.0).max(1.0)(_("std dev"), stdDev(),
                    [this](double value){
                        setStdDev(value);    
                        return true;
                    });

        putProperty.min(0.0).max(1.0)(_("salt"), salt(),
                    [this](double value){
                        setSalt(value);
                        return true;
                    });

        putProperty.min(0.0).max(1.0)(_("pepper"), pepper(),
                    [this](double value){
                        setPepper(value);
                        return true;
                    });
        break;
    default:
        break;
    }
}


bool MultiColliderItem::store(Archive& archive)
{
    if(!SimpleColliderItem::store(archive)) {
        return false;
    }
    archive.write("collider_type", colliderTypeSelection.selectedSymbol());

    // CFD
    archive.write("density", density());
    archive.write("viscosity", viscosity());
    write(archive, "steady_flow", Vector3(steadyFlow()));

    // TC
    archive.write("inbound_delay", inboundDelay());
    archive.write("inbound_rate", inboundRate());
    archive.write("inbound_loss", inboundLoss());
    archive.write("outbound_delay", outboundDelay());
    archive.write("outbound_rate", outboundRate());
    archive.write("outbound_loss", outboundLoss());
    archive.write("source", source());
    archive.write("destination", destination());

    // VFX
    write(archive, "hsv", Vector3(hsv()));
    write(archive, "rgb", Vector3(rgb()));
    archive.write("coef_b", coefB());
    archive.write("coef_d", coefD());
    archive.write("std_dev", stdDev());
    archive.write("salt", salt());
    archive.write("pepper", pepper());
    return true;
}


bool MultiColliderItem::restore(const Archive& archive)
{
    if(!SimpleColliderItem::restore(archive)) {
        return false;
    }
    string colliderId;
    if(archive.read("collider_type", colliderId)) {
        colliderTypeSelection.select(colliderId);
    }

    // CFD
    setDensity(archive.get("density", 0.0));
    setViscosity(archive.get("viscosity", 0.0));
    Vector3 v;
    if(read(archive, "steady_flow", v)) {
        setSteadyFlow(v);
    }

    // TC
    setInboundDelay(archive.get("inbound_delay", 0.0));
    setInboundRate(archive.get("inbound_rate", 0.0));
    setInboundLoss(archive.get("inbound_loss", 0.0));
    setOutboundDelay(archive.get("outbound_delay", 0.0));
    setOutboundRate(archive.get("outbound_rate", 0.0));
    setOutboundLoss(archive.get("outbound_loss", 0.0));
    setSource(archive.get("source", "0.0.0.0/0"));
    setDestination(archive.get("destination", "0.0.0.0/0"));

    // VFX
    if(read(archive, "hsv", v)) {
        setHsv(v);
    }
    if(read(archive, "rgb", v)) {
        setRgb(v);
    }
    setCoefB(archive.get("coef_b", 0.0));
    setCoefD(archive.get("coef_d", 1.0));
    setStdDev(archive.get("std_dev", 0.0));
    setSalt(archive.get("salt", 0.0));
    setPepper(archive.get("pepper", 0.0));
    return true;
}