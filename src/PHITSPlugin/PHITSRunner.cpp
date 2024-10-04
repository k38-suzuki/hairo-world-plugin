/**
   @author Kenta Suzuki
*/

#include "PHITSRunner.h"
#include <cnoid/Format>
#include <cnoid/UTF8>
#include <cnoid/stdx/filesystem>
#include <QTextStream>
#include <QThread>
#include "ComptonCone.h"
#include "GammaData.h"
#include "gettext.h"
#include <iostream>
#include "gettext.h"

using namespace std;
using namespace cnoid;
namespace filesystem = cnoid::stdx::filesystem;

PHITSRunner::PHITSRunner()
    : mv_(MessageView::instance())
{
    mode_ = GammaData::DOSERATE;
    filename_ = "./output_gamma.out";
    isReadStandardOutput_ = false;
    putMessages_ = true;
    isPHITS = true;

    process_.sigReadyReadStandardOutput().connect([&](){ onReadyReadStandardOutput(); });
    QObject::connect(&process_, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                     [this](int exitCode, QProcess::ExitStatus exitStatus){ onProcessFinished(exitCode, exitStatus); });
}


PHITSRunner::~PHITSRunner()
{
    process_.kill();
}


void PHITSRunner::startPHITS(std::string filename)
{
    isPHITS = true;
    filesystem::path path(fromUTF8(filename));
    QStringList arguments;
    arguments << filename.c_str();
    process_.setWorkingDirectory(path.parent_path().string().c_str());
    process_.start(PHITS_CMD, arguments);
    mv_->putln(_("PHITS has been executed."));
}


void PHITSRunner::startQAD(std::string inputfile, std::string outputfile)
{
    isPHITS = false;
    filesystem::path path(fromUTF8(inputfile));
    QStringList arguments;
    arguments << inputfile.c_str() << outputfile.c_str();
    process_.setWorkingDirectory(path.parent_path().string().c_str());
    process_.start(QAD_CMD, arguments);
    mv_->putln(_("QAD has been executed."));
}


void PHITSRunner::stop()
{
   process_.terminate();
   QThread::msleep(1000);
   process_.kill(); // Make sure we are really killing the phits process.
}


void PHITSRunner::setReadStandardOutput(const std::string& filename, const int& mode)
{
    isReadStandardOutput_ = true;
    filename_ = filename;
    mode_ = mode;
}


string PHITSRunner::installPath() const
{
    return QProcessEnvironment::systemEnvironment().value("PHITSPATH", PHITS_DEFAULT_LOC).toStdString();
}


void PHITSRunner::onReadyReadStandardOutput()
{
    process_.setReadChannel(QProcess::StandardOutput);
    QTextStream stream(&process_);
    while(!stream.atEnd()) {
        string line = stream.readLine().toStdString();
        if(putMessages_) {
            mv_->putln("# " + line);
            mv_->flush();
        }
        if(mode_ == GammaData::PINHOLE || mode_ == GammaData::COMPTON) {
            if(isReadStandardOutput_ && line.find("] ncas =") != string::npos) {
                readPHITSData(); //Read phits data if std out contained  substr "] ncas ="
            }
        }
    }
}


bool PHITSRunner::readPHITSData()
{
    bool result = false;
    switch (mode_) {
    case GammaData::PINHOLE:
        if(pcamera_) {
            result = loadGammaData(filename_, pcamera_);
        }
        break;
    case GammaData::COMPTON:
        if(ccamera_) {
            result = ComptonCone::readComptonCone(filename_, energy_, ccamera_);
            string filename = filename_ + ".tmp";
            result &= loadGammaData(filename, ccamera_);
        }
        break;
    default:
        sigReadPHITSData_(filename_);
        result = true;
    }
    return  result;
}


bool PHITSRunner::loadGammaData(const string& filename, GammaCamera* camera)
{
    bool isReady = false;
    GammaData& gammaData = camera->gammaData();
    if(gammaData.readPHITS(filename, camera->dataType())) {
        string name = filename + ".gbin";
        if(gammaData.write(name)) {
            gammaData.setDataHeaderInfo(gammaData.geometryInfo(0));
            isReady = true;
        }
    }
    camera->setReady(isReady);
    return isReady;
}


void PHITSRunner::onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    if(exitStatus != QProcess::NormalExit || exitCode != 0) {
        // Bad exit
        if(isPHITS) {
            mv_->putln(formatR(_("PHITS has been terminated. {0}"), exitCode));
        } else {
            mv_->putln(formatR(_("QAD has been terminated. {0}"), exitCode));
        }
    } else {
        //Ended naturally
        if(isPHITS) {
            mv_->putln(_("PHITS has been finished."));
        } else {
            mv_->putln(_("QAD has been finished."));
        }
        readPHITSData();
    }
    mv_->flush();
    sigProcessFinished_();
}


void PHITSRunner::setCamera(Camera* camera)
{
    ccamera_ = dynamic_cast<ComptonCamera*>(camera);
    pcamera_ = dynamic_cast<PinholeCamera*>(camera);
}


void PHITSRunner::putMessages(bool checked)
{
    putMessages_ = checked;
}
