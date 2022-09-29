/**
   \file
   \author Kenta Suzuki
*/

#ifndef CNOID_PHITSPLUGIN_PHITSWRITER_H
#define CNOID_PHITSPLUGIN_PHITSWRITER_H

#include <cnoid/Camera>
#include <cnoid/EigenUtil>
#include <string>
#include <vector>
#include "ComptonCamera.h"
#include "ConfigTable.h"
#include "PinholeCamera.h"

namespace cnoid {

class PHITSWriter
{
public:
    PHITSWriter();
    virtual ~PHITSWriter();

    virtual std::string writePHITS(GammaData::CalcInfo calcInfo);
    double energy() { return energy_; }
    void setCamera(Camera* camera);

    void setDefaultNuclideTableFile(const std::string& filename);
    void setDefaultElementTableFile(const std::string& filename);

protected:
    std::vector<double> materialRho;
    NuclideTable nuclideTable;
    ElementTable elementTable;

    // RadiationSource Parameter
    std::vector<int> nEne;
    std::vector< std::vector<double> > dEnergy;
    std::vector< std::vector<double> > dRate;
    std::vector< std::vector<double> > dActivity;
    std::vector<double> srcTotalActivity;
    std::vector<double> srcW, srcD, srcH;    // Witdh(x), Depth(y), Height(z) of Sources
    std::vector<double> srcVolume;
    std::vector<double> srcCX, srcCY, srcCZ; // Central coordinate (X, Y, Z) of Sources
    std::vector<Matrix3> srcRotMat;
    std::vector<std::string> strSrcShape;
    std::vector<int> srcMaterialId;
    int nSource;
    // only used in QAD
    std::vector<int> LSO;
    std::vector<int> MSO;
    std::vector<int> NSO;
    std::vector<std::string> buildupName;

    // Obstacles Parameter
    std::vector<double> obsW, obsD, obsH;    // Witdh(x), Depth(y), Height(z) of Obstacles
    std::vector<double> obsCX, obsCY, obsCZ; // Central coordinate (X, Y, Z) of Obstacles
    std::vector<Matrix3> obsRotMat;
    std::vector<std::string> strObsShape;
    std::vector<int> obsMaterialId;
    int nObstacle;

    // Common Camera Parameter
    double detCX, detCY, detCZ;
    Matrix3 detRotMat;
    float detRpy[3];
    Vector2 resolution;
    int detMaterialId;
    double detMaterialRho;

    // Pinhole Camera Parameter
    double detMatThickness;
    double detPinholeOpening;
    double detectorSize = 10; // unit: cm
    double shieldThickness;
    double angle; // radian
    double pinholeOpening;
    double distance;

    // Compton Camera Parameter
    double energy_;
    double detElementWidth;
    double detScattererThickness;
    double detAbsorberThickness;
    double detDistance;
    double detArm;
    const double elementDistance = 0.06;
    const double airThicknes = 0.3;
    double xminCC;
    double xmaxCC;
    double zminCC;
    double zmaxCC;
    double xminEU;
    double xmaxEU;
    double zminEU;
    double zmaxEU;
    double xminRU;
    double xmaxRU;
    double zminRU;
    double zmaxRU;
    double yminSC;
    double ymaxSC;
    double ymaxAB;
    double yminAB;
    double yairSC;
    double yairAB;

    ComptonCamera* ccamera;
    PinholeCamera* pcamera;
    std::string defaultNuclideTableFile_;
    std::string defaultElementTableFile_;

    // Function
    void initialize();
    bool searchLink(bool flagQAD = false);
    bool searchCameraLink(const int inputMode);
};

}

#endif
