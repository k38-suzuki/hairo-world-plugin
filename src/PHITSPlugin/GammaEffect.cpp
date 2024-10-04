/**
   @author Kenta Suzuki
*/

#include "GammaEffect.h"
#include <cnoid/ExecutablePath>
#include <cnoid/UTF8>
#include <cnoid/stdx/filesystem>
#include <QDateTime>
#include <QFile>
#include <QTextStream>
#include "ComptonCamera.h"
#include "PinholeCamera.h"

#ifndef _MSC_VER
#include <libgen.h>
#endif

using namespace std;
using namespace cnoid;
namespace filesystem = cnoid::stdx::filesystem;

namespace {

bool writeTextFile(const string& filename, const string& text)
{
    if(!text.empty()) {
        QFile file(filename.c_str());
        if(!file.open(QIODevice::WriteOnly)) {
            return false;
        }
        QTextStream qts(&file);
        qts << text.c_str();
        file.close();
    }
    return true;
}

}


GammaEffect::GammaEffect(Camera* camera)
    : camera_(camera),
      energyFilter_(new EnergyFilter)
{
    maxcas_ = 1000;
    maxbch_ = 2;
    is_message_checked_ = true;

    default_nuclide_table_file_ = toUTF8((shareDirPath() / "default" / "nuclides.yaml").string());
    default_element_table_file_ = toUTF8((shareDirPath() / "default" / "elements.yaml").string());
    default_energy_filter_file_ = toUTF8((shareDirPath() / "default" / "filters.yaml").string());

    phitsRunner_.setCamera(camera);
    phitsWriter_.setCamera(camera);
    energyFilter_->load(default_energy_filter_file_);
}


GammaEffect::~GammaEffect()
{

}


void GammaEffect::start(bool checked)
{
    ComptonCamera* comptonCamera = dynamic_cast<ComptonCamera*>(camera_);
    PinholeCamera* pinholeCamera = dynamic_cast<PinholeCamera*>(camera_);

    if(checked) {
        filesystem::path homeDirPath(fromUTF8(getenv("HOME")));
        QDateTime recordingStartTime = QDateTime::currentDateTime();
        string suffix = recordingStartTime.toString("-yyyy-MM-dd-hh-mm-ss").toStdString();
        string phits_dir = toUTF8((homeDirPath / "phits_ws" / ("phits" + suffix).c_str()).string());
        filesystem::path phitsDirPath(fromUTF8(phits_dir));
        if(!filesystem::exists(phitsDirPath)) {
            filesystem::create_directories(phitsDirPath);
        }

        if(comptonCamera || pinholeCamera) {
            string filename = toUTF8((phitsDirPath / "phits.inp").string());
            filesystem::path filePath(fromUTF8(filename));
            filesystem::path parentDirPath(filePath.parent_path());

            string filename0;
            GammaData::CalcInfo calcInfo;
            calcInfo.maxcas = maxcas_;
            calcInfo.maxbch = maxbch_;
            if(comptonCamera) {
                calcInfo.inputMode = GammaData::COMPTON;
                filename0 = toUTF8((parentDirPath / "flux_cross_dmp.out").string());
            } else if(pinholeCamera) {
                calcInfo.inputMode = GammaData::PINHOLE;
                filename0 = toUTF8((parentDirPath / "cross_xz.out").string());
            }

            phitsWriter_.setDefaultNuclideTableFile(default_nuclide_table_file_);
            phitsWriter_.setDefaultElementTableFile(default_element_table_file_);
            writeTextFile(filename, phitsWriter_.writePHITS(calcInfo));
            phitsRunner_.setEnergy(phitsWriter_.energy());
            phitsRunner_.setReadStandardOutput(filename0, calcInfo.inputMode);
            phitsRunner_.startPHITS(filename.c_str());
        }
    } else {
        if(comptonCamera || pinholeCamera) {
            phitsRunner_.stop();
        }
    }
}
