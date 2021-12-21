/**
   \file
   \author Kenta Suzuki
*/

#ifndef CNOID_FLUIDDYNAMICSPLUGIN_CFDBODY_H
#define CNOID_FLUIDDYNAMICSPLUGIN_CFDBODY_H

#include <cnoid/Body>
#include "CFDLink.h"

namespace cnoid {

class CFDBodyImpl;

class CFDBody : public Referenced
{
public:
    CFDBody();
    CFDBody(Body* body);
    virtual ~CFDBody();

    Body* body() const;
    CFDLink* cfdLink(const int& index);
    size_t numFDLinks() const;

    void addFDLinks(CFDLink* cfdLink);

private:
    CFDBodyImpl* impl;
    friend class CFDBodyImpl;
};

typedef ref_ptr<CFDBody> CFDBodyPtr;

}

#endif // CNOID_FLUIDDYNAMICSPLUGIN_CFDBODY_H
