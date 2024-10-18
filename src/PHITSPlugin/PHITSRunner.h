/**
   @author Kenta Suzuki
*/

#ifndef CNOID_PHITS_PLUGIN_PHITS_RUNNER_H
#define CNOID_PHITS_PLUGIN_PHITS_RUNNER_H

#include <cnoid/MessageView>
#include <cnoid/Process>
#include <cnoid/Signal>
#include "ComptonCamera.h"
#include "PinholeCamera.h"

/**
PHITS command for each OS environment.
  */
#ifdef _MSC_VER
//#define PHITS_CMD  "cmd /c phits"
#define PHITS_CMD "phits.bat"
#define PHITS_DEFAULT_LOC "c:/phits"
#else
#define PHITS_CMD "phits.sh"
#define PHITS_DEFAULT_LOC "~/phits"
#endif
#define QAD_CMD "QAD-CGGP2R"

namespace cnoid {

class PHITSRunner
{
public:
    PHITSRunner();
    virtual ~PHITSRunner();

    void startPHITS(std::string filename);
    void startQAD(std::string inputfile, std::string outputfile);
    void stop();
    void setReadStandardOutput(const std::string& filename, const int& mode);
    void setEnergy(const double energy) { energy_ = energy; }
    double energy() const { return energy_; }
    std::string installPath() const;
    void setCamera(Camera* camera);
    void putMessages(bool checked);

    SignalProxy<void(const std::string& filename)> sigReadPHITSData() { return sigReadPHITSData_; }
    SignalProxy<void()> sigProcessFinished() { return sigProcessFinished_; }

private:
    bool isReadStandardOutput_;
    Process process_;
    int mode_;
    std::string filename_;
    double energy_;
    ComptonCamera* ccamera_;
    PinholeCamera* pcamera_;
    MessageView* mv_;
    bool putMessages_;
    bool isPHITS;

    Signal<void(const std::string& filename)> sigReadPHITSData_;
    Signal<void()> sigProcessFinished_;

    void onReadyReadStandardOutput();
    void onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
    bool readPHITSData();
    bool loadGammaData(const std::string& filename, GammaCamera* camera);
};

}

#endif // CNOID_PHITS_PLUGIN_PHITS_RUNNER_H
