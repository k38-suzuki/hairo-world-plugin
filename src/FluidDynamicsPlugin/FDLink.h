/**
   \file
   \author Kenta Suzuki
*/

#ifndef CNOID_FLUID_DYNAMICS_PLUGIN_FD_LINK_H
#define CNOID_FLUID_DYNAMICS_PLUGIN_FD_LINK_H

#include <cnoid/EigenTypes>
#include <cnoid/Link>

namespace cnoid {

class FDLinkImpl;

class FDLink : public Referenced
{
public:
    FDLink();
    FDLink(Link* link);
    virtual ~FDLink();

    Link* link() const;

    void setDensity(const double density);
    double density() const;
    void setCenterOfBuoyancy(const Vector3 centerOfBuoyancy);
    Vector3 centerOfBuoyancy() const;
    void setCdw(const double cdw);
    double cdw() const;
    void setCda(const double cda);
    double cda() const;
    void setTd(const double td);
    double td() const;
    void setSurface(const Vector6 surface);
    Vector6 surface() const;
    void setCv(const double cv);
    double cv() const;

private:
    FDLinkImpl* impl;
    friend class FDLinkImpl;
};

typedef ref_ptr<FDLink> FDLinkPtr;

}

#endif // CNOID_FLUID_DYNAMICS_PLUGIN_FD_LINK_H
