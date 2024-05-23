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

    Process* myProcess;
    Signal<void(string text)> sigDecoded;

    void decode(const string& filename);
    void onReadyReadStandardOutput();
};

}


QRDecoder::QRDecoder()
{
    impl = new Impl;
}


QRDecoder::Impl::Impl()
{
    myProcess = new Process;
    Process::connect(myProcess, &Process::readyReadStandardOutput,
        [=](){ onReadyReadStandardOutput(); });
}


QRDecoder::~QRDecoder()
{
    delete impl;
}


QRDecoder::Impl::~Impl()
{
    if(myProcess->state() != Process::NotRunning) {
        myProcess->kill();
        myProcess->waitForFinished(100);
    }
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
        myProcess->start(program, arguments);
    }
}


void QRDecoder::Impl::onReadyReadStandardOutput()
{
    QString text(myProcess->readAllStandardOutput());
    text.replace("QR-Code:", "");
    text.replace("\n", "");
    sigDecoded(text.toStdString());
}


SignalProxy<void(string text)> QRDecoder::sigDecoded()
{
    return impl->sigDecoded;
}