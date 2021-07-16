/**
   \file
   \author Kenta Suzuki
*/

#ifndef CNOID_STARTUPPLUGIN_JPROCESS_H
#define CNOID_STARTUPPLUGIN_JPROCESS_H

#include <cnoid/Process>
#include <cnoid/Signal>

namespace cnoid {

class JProcess : public Process
{
    Q_OBJECT

public:
    JProcess(QObject* parent = 0);

    SignalProxy<void(int exitCode, QProcess::ExitStatus exitStatus)> sigFinished() {
        return sigFinished_;
    }

private Q_SLOTS:
    void onFinished(int exitCode, QProcess::ExitStatus exitStatus);

private:
    Signal<void(int exitCode, QProcess::ExitStatus exitStatus)> sigFinished_;
};

}

#endif // CNOID_STARTUPPLUGIN_JPROCESS_H
