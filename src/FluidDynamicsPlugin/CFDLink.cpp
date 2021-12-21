/**
   \file
   \author Kenta Suzuki
*/

#include "CFDLink.h"

using namespace cnoid;

CFDLink::CFDLink()
{
    link_ = nullptr;
    initialize();
}


CFDLink::CFDLink(Link* link)
    : link_(link)
{
    initialize();
}


CFDLink::~CFDLink()
{

}


void CFDLink::initialize()
{
    density_ = 0.0;
    centerOfBuoyancy_ << 0.0, 0.0, 0.0;
    cdw_ = 0.0;
    cda_ = 0.0;
    td_ = 0.0;
    surface_ << 0.0, 0.0, 0.0, 0.0, 0.0, 0.0;
    cv_ = 0.0;
}
