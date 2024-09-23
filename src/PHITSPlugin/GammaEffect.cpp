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
        filesystem::path homeDir(getenv("HOME"));
        QDateTime recordingStartTime = QDateTime::currentDateTime();
        string suffix = recordingStartTime.toString("-yyyy-MM-dd-hh-mm-ss").toStdString();
        string phitsDirPath = toUTF8((homeDir / "phits_ws" / ("phits" + suffix).c_str()).string());
        filesystem::path dir(fromUTF8(phitsDirPath));
        if(!filesystem::exists(dir)) {
            filesystem::create_directories(dir);
        }

        if(comptonCamera || pinholeCamera) {
            string filename = toUTF8((dir / "phits").string()) + ".inp";
            filesystem::path filePath(filename);
            filesystem::path dir(filePath.parent_path());

            string filename0;
            GammaData::CalcInfo calcInfo;
            calcInfo.maxcas = maxcas_;
            calcInfo.maxbch = maxbch_;
            if(comptonCamera) {
                calcInfo.inputMode = GammaData::COMPTON;
                filename0 = toUTF8((dir / "flux_cross_dmp.out").string());
            } else if(pinholeCamera) {
                calcInfo.inputMode = GammaData::PINHOLE;
                filename0 = toUTF8((dir / "cross_xz.out").string());
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
