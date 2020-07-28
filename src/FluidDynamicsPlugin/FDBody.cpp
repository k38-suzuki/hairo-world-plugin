/**
   \file
   \author Kenta Suzuki
*/

#include "FDBody.h"

using namespace cnoid;

namespace cnoid {

class FDBodyImpl
{
public:
    FDBodyImpl(FDBody* self);
    FDBodyImpl(FDBody* self, Body* body);

    FDBody* self;
    Body* body;
    std::vector<FDLink*> fdLinks;
    void initialize();
};

}


FDBody::FDBody()
{
    impl = new FDBodyImpl(this);
}


FDBodyImpl::FDBodyImpl(FDBody* self)
    : self(self)
{
    body = new Body();
    initialize();
}


FDBody::FDBody(Body* body)
    : impl(new FDBodyImpl(this, body))
{

}


FDBodyImpl::FDBodyImpl(FDBody* self, Body* body)
    : self(self)
{
    this->body = body;
    initialize();
}


FDBody::~FDBody()
{
    delete impl;
}

Body* FDBody::body() const
{
    return impl->body;
}


FDLink* FDBody::fdLink(const int Index)
{
    return impl->fdLinks[Index];
}


size_t FDBody::numFDLinks() const
{
    return impl->fdLinks.size();
}


void FDBody::addFDLinks(FDLink* fdLink)
{
    impl->fdLinks.push_back(fdLink);
}


void FDBodyImpl::initialize()
{
    fdLinks.clear();
}
