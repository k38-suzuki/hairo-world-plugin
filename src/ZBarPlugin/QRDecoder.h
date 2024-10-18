/**
   @author Kenta Suzuki
*/

#ifndef CNOID_ZBAR_PLUGIN_QR_DECODER_H
#define CNOID_ZBAR_PLUGIN_QR_DECODER_H

#include <cnoid/Signal>
#include "exportdecl.h"

namespace cnoid {

class CNOID_EXPORT QRDecoder
{
public:

    QRDecoder();
    virtual ~QRDecoder();

    void decode(const std::string& filename);

    SignalProxy<void(const std::string& text)> sigDecoded();

private:
    class Impl;
    Impl* impl;
};

}

#endif // CNOID_ZBAR_PLUGIN_QR_DECODER_H