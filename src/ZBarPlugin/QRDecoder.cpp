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

    void decode(const string& filename) const;
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

}


void QRDecoder::decode(const string& filename) const
{
    impl->decode(filename);
}


void QRDecoder::Impl::decode(const string& filename) const
{
    QString program = "zbarimg";
    QStringList arguments;
    arguments << "--nodbus" << "--quiet" << filename.c_str();
    myProcess->start(program, arguments);
}


void QRDecoder::Impl::onReadyReadStandardOutput()
{
    QString output(myProcess->readAllStandardOutput());
    output.replace("QR-Code:", "");
    output.replace("\n", "");
    sigDecoded(output.toStdString());
}


SignalProxy<void(string text)> QRDecoder::sigDecoded()
{
    return impl->sigDecoded;
}