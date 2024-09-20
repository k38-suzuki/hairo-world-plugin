/**
    @author Kenta Suzuki
*/

#ifndef CNOID_ZBAR_PLUGIN_QR_READER_H
#define CNOID_ZBAR_PLUGIN_QR_READER_H

namespace cnoid {

class ExtensionManager;

class QRReader
{
public:
    static void initializeClass(ExtensionManager* ext);

    QRReader();
    virtual ~QRReader();

private:
    class Impl;
    Impl* impl;
};

}

#endif // CNOID_ZBAR_PLUGIN_QR_READER_H