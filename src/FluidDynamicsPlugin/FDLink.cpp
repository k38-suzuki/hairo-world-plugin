/**
   \file
   \author Kenta Suzuki
*/

#include "FDLink.h"

using namespace cnoid;

namespace cnoid {

class FDLinkImpl
{
public:
    FDLinkImpl(FDLink* self);
    FDLinkImpl(FDLink* self, Link* link);

    FDLink* self;
    Link* link;
    double density;
    Vector3 centerOfBuoyancy;
    double cdw;
    double cda;
    double td;
    Vector6 surface;
    double cv;

    void initialize();
};

}


FDLink::FDLink()
{
    impl = new FDLinkImpl(this);
}


FDLinkImpl::FDLinkImpl(FDLink* self)
    : self(self)
{
    link = new Link();
    initialize();
}


FDLink::FDLink(Link* link)
    : impl(new FDLinkImpl(this, link))
{

}


FDLinkImpl::FDLinkImpl(FDLink* self, Link* link)
    : self(self)
{
    this->link = link;
    initialize();
}


FDLink::~FDLink()
{
    delete impl;
}


Link* FDLink::link() const
{
    return impl->link;
}


void FDLink::setDensity(const double& density)
{
    impl->density = density;
}


double FDLink::density() const
{
    return impl->density;
}


void FDLink::setCenterOfBuoyancy(const Vector3& centerOfBuoyancy)
{
    impl->centerOfBuoyancy = centerOfBuoyancy;
}


Vector3 FDLink::centerOfBuoyancy() const
{
    return impl->centerOfBuoyancy;
}


void FDLink::setCdw(const double& cdw)
{
    impl->cdw = cdw;
}


double FDLink::cdw() const
{
    return impl->cdw;
}


void FDLink::setCda(const double& cda)
{
    impl->cda = cda;
}


double FDLink::cda() const
{
    return impl->cda;
}


void FDLink::setTd(const double& td)
{
    impl->td = td;
}


double FDLink::td() const
{
    return impl->td;
}


void FDLink::setSurface(const Vector6& surface)
{
    impl->surface = surface;
}


Vector6 FDLink::surface() const
{
    return impl->surface;
}


void FDLink::setCv(const double& cv)
{
    impl->cv = cv;
}


double FDLink::cv() const
{
    return impl->cv;
}


void FDLinkImpl::initialize()
{
    density = 0.0;
    centerOfBuoyancy << 0.0, 0.0, 0.0;
    cdw = 0.0;
    cda = 0.0;
    td = 0.0;
    surface << 0.0, 0.0, 0.0, 0.0, 0.0, 0.0;
    cv = 0.0;
}
