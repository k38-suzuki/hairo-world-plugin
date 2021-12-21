/**
   \file
   \author Kenta Suzuki
*/

#ifndef CNOID_FLUIDDYNAMICSPLUGIN_CFDLINK_H
#define CNOID_FLUIDDYNAMICSPLUGIN_CFDLINK_H

#include <cnoid/EigenTypes>
#include <cnoid/Link>

namespace cnoid {

class CFDLink : public Referenced
{
public:
    CFDLink();
    CFDLink(Link* link);
    virtual ~CFDLink();

    Link* link() const { return link_; }
    void setDensity(const double& density) { density_ = density; }
    double density() const { return density_; }
    void setCenterOfBuoyancy(const Vector3& centerOfBuoyancy) { centerOfBuoyancy_ = centerOfBuoyancy; }
    Vector3 centerOfBuoyancy() const { return centerOfBuoyancy_; }
    void setCdw(const double& cdw) { cdw_ = cdw; }
    double cdw() const { return cdw_; }
    void setCda(const double& cda) { cda_ = cda; }
    double cda() const { return cda_; }
    void setTd(const double& td) { td_ = td; }
    double td() const { return td_; }
    void setSurface(const Vector6& surface) { surface_ = surface; }
    Vector6 surface() const { return surface_; }
    void setCv(const double& cv) { cv_ = cv; }
    double cv() const { return cv_; }

private:
    Link* link_;
    double density_;
    Vector3 centerOfBuoyancy_;
    double cdw_;
    double cda_;
    double td_;
    Vector6 surface_;
    double cv_;

    void initialize();
};

typedef ref_ptr<CFDLink> CFDLinkPtr;

}

#endif // CNOID_FLUIDDYNAMICSPLUGIN_CFDLINK_H
