/**
   \file
   \author Kenta Suzuki
*/

#ifndef CNOID_PHITSPLUGIN_QADWRITER_H
#define CNOID_PHITSPLUGIN_QADWRITER_H

#include "PHITSWriter.h"

namespace cnoid {

class QADWriter : public PHITSWriter
{
public:
    QADWriter();
    virtual ~QADWriter();

    std::string writeQAD(GammaData::CalcInfo& calcInfo, int iSrc);
};

}

#endif
