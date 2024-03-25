/**
   @author Kenta Suzuki
*/

#ifndef CNOID_PHITS_PLUGIN_QAD_WRITER_H
#define CNOID_PHITS_PLUGIN_QAD_WRITER_H

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

#endif // CNOID_PHITS_PLUGIN_QAD_WRITER_H
