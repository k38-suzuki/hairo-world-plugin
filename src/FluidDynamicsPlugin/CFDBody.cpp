/**
   \file
   \author Kenta Suzuki
*/

#include "CFDBody.h"
#include <vector>

using namespace cnoid;
using namespace std;

namespace cnoid {

class CFDBodyImpl
{
public:
    CFDBodyImpl(CFDBody* self);
    CFDBodyImpl(CFDBody* self, Body* body);
    CFDBody* self;

    Body* body;
    vector<CFDLink*> cfdLinks;

    void initialize();
};

}


CFDBody::CFDBody()
{
    impl = new CFDBodyImpl(this);
}


CFDBodyImpl::CFDBodyImpl(CFDBody* self)
    : self(self)
{
    body = nullptr;
    initialize();
}


CFDBody::CFDBody(Body* body)
    : impl(new CFDBodyImpl(this, body))
{

}


CFDBodyImpl::CFDBodyImpl(CFDBody* self, Body* body)
    : self(self),
      body(body)
{
    initialize();
}


CFDBody::~CFDBody()
{
    delete impl;
}

Body* CFDBody::body() const
{
    return impl->body;
}


CFDLink* CFDBody::cfdLink(const int& index)
{
    return impl->cfdLinks[index];
}


size_t CFDBody::numFDLinks() const
{
    return impl->cfdLinks.size();
}


void CFDBody::addFDLinks(CFDLink* cfdLink)
{
    impl->cfdLinks.push_back(cfdLink);
}


void CFDBodyImpl::initialize()
{
    cfdLinks.clear();
}
