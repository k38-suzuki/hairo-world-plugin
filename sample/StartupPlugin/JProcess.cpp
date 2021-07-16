/**
   \file
   \author Kenta Suzuki
*/

#include "JProcess.h"

using namespace cnoid;

JProcess::JProcess(QObject* parent)
    : Process(parent)
{
    connect(this, SIGNAL(finished(int, QProcess::ExitStatus)),
            this, SLOT(onFinished(int, QProcess::ExitStatus)));
}


void JProcess::onFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    sigFinished_(exitCode, exitStatus);
}
