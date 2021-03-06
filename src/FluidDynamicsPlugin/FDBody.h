/**
   \file
   \author Kenta Suzuki
*/

#ifndef CNOID_FLUID_DYNAMICS_PLUGIN_FD_BODY_H
#define CNOID_FLUID_DYNAMICS_PLUGIN_FD_BODY_H

#include <cnoid/Body>
#include "FDLink.h"

namespace cnoid {

class FDBodyImpl;

class FDBody : public Referenced
{
public:
    FDBody();
    FDBody(Body* body);
    virtual ~FDBody();

    Body* body() const;
    FDLink* fdLink(const int& Index);
    size_t numFDLinks() const;

    void addFDLinks(FDLink* fdLink);

private:
    FDBodyImpl* impl;
    friend class FDBodyImpl;
};

typedef ref_ptr<FDBody> FDBodyPtr;

}

#endif // CNOID_FLUID_DYNAMICS_PLUGIN_FD_BODY_H
