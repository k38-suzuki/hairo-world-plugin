/**
   @author Kenta Suzuki
*/

#include "QRDecoder.h"
#include <cnoid/Process>

using namespace std;
using namespace cnoid;

namespace cnoid {

class QRDecoder::Impl
{
public:

    Impl();
    ~Impl();

    Process process;
    Signal<void(const string& text)> sigDecoded;

    void decode(const string& filename);
    bool terminate();
    void onReadyReadStandardOutput();
};

}


QRDecoder::QRDecoder()
{
    impl = new Impl;
}


QRDecoder::Impl::Impl()
{
    process.sigReadyReadStandardOutput().connect(
        [&](){ onReadyReadStandardOutput(); });
}


QRDecoder::~QRDecoder()
{
    delete impl;
}


QRDecoder::Impl::~Impl()
{
    terminate();
}


void QRDecoder::decode(const string& filename)
{
    impl->decode(filename);
}


void QRDecoder::Impl::decode(const string& filename)
{
    if(!filename.empty()) {
        QString program = "zbarimg";
        QStringList arguments;
        arguments << "--nodbus" << "--quiet" << filename.c_str();

        terminate();
        process.start(program, arguments);

        if(process.waitForStarted()) {

        }
    }
}


bool QRDecoder::Impl::terminate()
{
    if(process.state() != Process::NotRunning) {
        process.kill();
        return process.waitForFinished(100);
    }
    return false;
}


void QRDecoder::Impl::onReadyReadStandardOutput()
{
    QString text(process.readAllStandardOutput());
    text.replace("QR-Code:", "");
    text.replace("\n", "");
    sigDecoded(text.toStdString());
}


SignalProxy<void(const string& text)> QRDecoder::sigDecoded()
{
    return impl->sigDecoded;
}