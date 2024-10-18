/**
   @author Kenta Suzuki
*/

#include "OrthoNodeData.h"
#include <cnoid/NullOut>
#include <cnoid/YAMLReader>
#include <array>
#include <tuple>
#include "gettext.h"

using namespace std;
using namespace cnoid;

namespace {

vector<tuple<string, double>> shields;

template<typename T> T linearInterpolateByLength(const T& val1, const T& val2, const T& len1, const T& len2)
{
    return (val2 - val1) / (len1 + len2) * len1 + val1;
}

template<typename T> T linearInterpolateByPosition(const T& val1, const T& val2, const T& pos, const T& pos1, const T& pos2)
{
    return (val2 - val1) / (pos2 - pos1) * (pos - pos1) + val1;
}

double interPolation(double x, double x2, double x1, double y2, double y1)
{
    double a = 0;
    double b = 0;
    double result = 0;

    a = (y2 - y1) / (x2 - x1);
    b = y2 - a * x2;
    result = a * x + b;
    return result;
}

class OrthoCellData
{
public:
    OrthoCellData();
    ~OrthoCellData();

    virtual bool isValid() const { return isValid_; }
    virtual size_t size(const int axis) const { return coordinates_[axis].size() - 1; }
    virtual vector<double> coordinates(const int axis) const { return coordinates_[axis]; }
    virtual double min() const { return *min_element(cell_values_.begin(), cell_values_.end()); }
    virtual double max() const { return *max_element(cell_values_.begin(), cell_values_.end()); }
    virtual double value(const uint32_t x, const uint32_t y, const uint32_t) const;
    virtual double value_shield(const int i, const uint32_t x, const uint32_t y, const uint32_t) const;

    bool createSampleData(const GammaData& gammaData);
    bool createShieldData(string& filename, const GammaData& gammaData);

private:
    bool isValid_;
    vector<double> coordinates_[OrthoNodeData::NumAxes];
    vector<double> cell_values_;
    vector<double> *cell_shield_values_;
    const double arrowableError_ = 1.0e-3;
};

}

namespace cnoid {

class ShieldTable
{
public:
    ShieldTable();
    ~ShieldTable();

    bool load(const std::string& filename, std::ostream& os = nullout());
    void clear();

    int nMat;
    int nEne;
    int nMFP;
    std::string*  sMat;
    double* sDensity;
    double** mAEne;
    double** mAVal;
    double** dTEne;
    double** dTMFP;
    double*** dTVal;
};

}


OrthoCellData::OrthoCellData()
{
    isValid_ = false;
}


OrthoCellData::~OrthoCellData()
{

}


double OrthoCellData::value(const uint32_t x, const uint32_t y, const uint32_t z) const
{
    return cell_values_[(coordinates_[OrthoNodeData::Y_AXIS].size()-1)*(coordinates_[OrthoNodeData::Z_AXIS].size()-1)*x+(coordinates_[OrthoNodeData::Z_AXIS].size()-1)*y+z];
}


double OrthoCellData::value_shield(const int i, const uint32_t x, const uint32_t y, const uint32_t z) const
{
    return cell_shield_values_[i][(coordinates_[OrthoNodeData::Y_AXIS].size() - 1)*(coordinates_[OrthoNodeData::Z_AXIS].size() - 1)*x + (coordinates_[OrthoNodeData::Z_AXIS].size() - 1)*y + z];
}


bool OrthoCellData::createSampleData(const GammaData& gammaData)
{
    if(!&gammaData) {
       return false;
    }

    //clear point data
    coordinates_[OrthoNodeData::X_AXIS].clear();
    coordinates_[OrthoNodeData::Y_AXIS].clear();
    coordinates_[OrthoNodeData::Z_AXIS].clear();
    cell_values_.clear();

    int directionNumber = gammaData.geometryInfo(0).calcDirectionNumber;
    GammaData::DataInfo dataInfo = gammaData.dataInfo();
    //TODO comment release
    if(dataInfo.calcDirectionRec.size() == 0 || gammaData.dataMode() == 0) {
        return false;
    }

    vector<float> origin = gammaData.geometryInfo(0).calcPoint;
    vector<float> calcPoint;
    calcPoint.resize(3);


    //get xyz coordinates_
    for(int i = 0; i < directionNumber; i++) {
        //cout<<i<<endl;
        GammaData::CalcDirectionRecInfo recInfo = dataInfo.calcDirectionRec[i];
        for(int ix=0;ix<2;ix++) {
            calcPoint[0]=origin[0]+pow(-1.0,ix)*recInfo.deltaX*0.5+recInfo.directionX;
            if(coordinates_[OrthoNodeData::X_AXIS].size() > 0) {
                bool xFlag=true;
                for(int j = 0; j < coordinates_[OrthoNodeData::X_AXIS].size(); j++) {
                    if(fabs(coordinates_[OrthoNodeData::X_AXIS][j]-calcPoint[0]) < arrowableError_) {
                        xFlag=false;
                        break;
                    }
                }
                if(xFlag == true) {
                    coordinates_[OrthoNodeData::X_AXIS].push_back(calcPoint[0]);
                }
            } else {
                coordinates_[OrthoNodeData::X_AXIS].push_back(calcPoint[0]);
            }
        }

        for(int iy=0;iy<2;iy++) {
            calcPoint[1]=origin[1]+pow(-1.0,iy)*recInfo.deltaY*0.5+recInfo.directionY;
            if(coordinates_[OrthoNodeData::Y_AXIS].size()>0) {
                bool yFlag = true;
                for(int j = 0; j < coordinates_[OrthoNodeData::Y_AXIS].size(); j++) {
                    if(fabs(coordinates_[OrthoNodeData::Y_AXIS][j]-calcPoint[1]) < arrowableError_) {
                        yFlag=false;
                        break;
                    }
                }
                if(yFlag == true) {
                    coordinates_[OrthoNodeData::Y_AXIS].push_back(calcPoint[1]);
                }
            } else {
                coordinates_[OrthoNodeData::Y_AXIS].push_back(calcPoint[1]);
            }
        }

        for(int iz=0;iz<2;iz++) {
            calcPoint[2]=origin[2]+pow(-1.0,iz)*recInfo.deltaZ*0.5+recInfo.directionZ;
            if(coordinates_[OrthoNodeData::Z_AXIS].size()>0) {
                bool zFlag=true;
                for(int j = 0; j < coordinates_[OrthoNodeData::Z_AXIS].size(); j++) {
                    if(fabs(coordinates_[OrthoNodeData::Z_AXIS][j]-calcPoint[2]) < arrowableError_) {
                        zFlag=false;
                        break;
                    }
                }
                if(zFlag == true) {
                    coordinates_[OrthoNodeData::Z_AXIS].push_back(calcPoint[2]);
                }
            } else {
                coordinates_[OrthoNodeData::Z_AXIS].push_back(calcPoint[2]);
            }
        }
    }

    //sort
    sort(coordinates_[OrthoNodeData::X_AXIS].begin(),coordinates_[OrthoNodeData::X_AXIS].end());
    sort(coordinates_[OrthoNodeData::Y_AXIS].begin(),coordinates_[OrthoNodeData::Y_AXIS].end());
    sort(coordinates_[OrthoNodeData::Z_AXIS].begin(),coordinates_[OrthoNodeData::Z_AXIS].end());

    //create _celValueAry
    int cellValueArySize=(coordinates_[OrthoNodeData::X_AXIS].size()-1)*(coordinates_[OrthoNodeData::Y_AXIS].size()-1)*(coordinates_[OrthoNodeData::Z_AXIS].size()-1);
    cell_values_.resize(cellValueArySize);
    int xNum=0;
    int yNum=0;
    int zNum=0;
    for(int i = 0; i < directionNumber; i++) {
        //cout<<i<<endl;
        GammaData::CalcDirectionRecInfo recInfo = dataInfo.calcDirectionRec[i];
        xNum=yNum=zNum=0;
        calcPoint[0]=origin[0]+recInfo.directionX;
        calcPoint[1]=origin[1]+recInfo.directionY;
        calcPoint[2]=origin[2]+recInfo.directionZ;

        for(int j = 0; j < coordinates_[OrthoNodeData::X_AXIS].size() - 1; j++) {
            if(fabs((coordinates_[OrthoNodeData::X_AXIS][j]+coordinates_[OrthoNodeData::X_AXIS][j+1])*0.5-calcPoint[0]) < arrowableError_) {
                xNum=j;
                break;
            }
        }

        for(int j = 0; j < coordinates_[OrthoNodeData::Y_AXIS].size() - 1; j++) {
            if(fabs((coordinates_[OrthoNodeData::Y_AXIS][j]+coordinates_[OrthoNodeData::Y_AXIS][j+1])*0.5-calcPoint[1]) < arrowableError_) {
                yNum=j;
                break;
            }
        }

        for(int j = 0; j < coordinates_[OrthoNodeData::Z_AXIS].size() - 1; j++) {
            if(fabs((coordinates_[OrthoNodeData::Z_AXIS][j]+coordinates_[OrthoNodeData::Z_AXIS][j+1])*0.5-calcPoint[2]) < arrowableError_) {
                zNum=j;
                break;
            }
        }
        for(int j = 0; j < gammaData.energySpectrumChannelNumber(); ++j) {
            cell_values_[(coordinates_[OrthoNodeData::Y_AXIS].size() - 1)*(coordinates_[OrthoNodeData::Z_AXIS].size() - 1)*xNum + (coordinates_[OrthoNodeData::Z_AXIS].size() - 1)*yNum + zNum] += recInfo.dirData[j] * gammaData.scaleFactor()*dataInfo.scaleFactor;
        }
    }
    isValid_ = true;
    return true;
}


bool OrthoCellData::createShieldData(string& filename, const GammaData& gammaData)
{
    if(!&gammaData) {
       return false;
    }

    //clear point data
    coordinates_[OrthoNodeData::X_AXIS].clear();
    coordinates_[OrthoNodeData::Y_AXIS].clear();
    coordinates_[OrthoNodeData::Z_AXIS].clear();

    int directionNumber = gammaData.geometryInfo(0).calcDirectionNumber;
    GammaData::DataInfo dataInfo = gammaData.dataInfo();
    //TODO comment release
    if(dataInfo.calcDirectionRec.size() == 0 || gammaData.dataMode() == 0) {
        return false;
    }

    vector<float> origin = gammaData.geometryInfo(0).calcPoint;
    vector<float> calcPoint;
    calcPoint.resize(3);

    ShieldTable shieldTable;
    shieldTable.load(filename);

    static const int nMat = shieldTable.nMat;
    static const int nEne = shieldTable.nEne;
    static const int nMFP = shieldTable.nMFP;

    double density = 1.0;
    double MFP;
    vector<double> mAEne(nEne, 0);
    vector<double> mAVal(nEne, 0);
    vector<double> dTEne(nEne, 0);
    vector<double> dTMFP(nMFP, 0);
    vector<vector<double>> dTVal(nEne, vector<double>(nMFP, 0));

    //get xyz coordinates_
    for(int i = 0; i < directionNumber; i++) {
        //cout<<i<<endl;
        GammaData::CalcDirectionRecInfo recInfo = dataInfo.calcDirectionRec[i];
        for(int ix = 0; ix < 2; ix++) {
            calcPoint[0] = origin[0] + pow(-1.0, ix)*recInfo.deltaX*0.5 + recInfo.directionX;
            if(coordinates_[OrthoNodeData::X_AXIS].size() > 0) {
                bool xFlag = true;
                for(int j = 0; j < coordinates_[OrthoNodeData::X_AXIS].size(); j++) {
                    if(fabs(coordinates_[OrthoNodeData::X_AXIS][j] - calcPoint[0]) < arrowableError_) {
                        xFlag = false;
                        break;
                    }
                }
                if(xFlag == true) {
                    coordinates_[OrthoNodeData::X_AXIS].push_back(calcPoint[0]);
                }
            } else {
                coordinates_[OrthoNodeData::X_AXIS].push_back(calcPoint[0]);
            }
        }

        for(int iy = 0; iy < 2; iy++) {
            calcPoint[1] = origin[1] + pow(-1.0, iy)*recInfo.deltaY*0.5 + recInfo.directionY;
            if(coordinates_[OrthoNodeData::Y_AXIS].size() > 0) {
                bool yFlag = true;
                for(int j = 0; j < coordinates_[OrthoNodeData::Y_AXIS].size(); j++) {
                    if(fabs(coordinates_[OrthoNodeData::Y_AXIS][j] - calcPoint[1]) < arrowableError_) {
                        yFlag = false;
                        break;
                    }
                }
                if(yFlag == true) {
                    coordinates_[OrthoNodeData::Y_AXIS].push_back(calcPoint[1]);
                }
            } else {
                coordinates_[OrthoNodeData::Y_AXIS].push_back(calcPoint[1]);
            }
        }

        for(int iz = 0; iz < 2; iz++) {
            calcPoint[2] = origin[2] + pow(-1.0, iz)*recInfo.deltaZ*0.5 + recInfo.directionZ;
            if(coordinates_[OrthoNodeData::Z_AXIS].size() > 0) {
                bool zFlag = true;
                for(int j = 0; j < coordinates_[OrthoNodeData::Z_AXIS].size(); j++) {
                    if(fabs(coordinates_[OrthoNodeData::Z_AXIS][j] - calcPoint[2]) < arrowableError_) {
                        zFlag = false;
                        break;
                    }
                }
                if(zFlag == true) {
                    coordinates_[OrthoNodeData::Z_AXIS].push_back(calcPoint[2]);
                }
            } else {
                coordinates_[OrthoNodeData::Z_AXIS].push_back(calcPoint[2]);
            }
        }
    }

    //sort
    sort(coordinates_[OrthoNodeData::X_AXIS].begin(), coordinates_[OrthoNodeData::X_AXIS].end());
    sort(coordinates_[OrthoNodeData::Y_AXIS].begin(), coordinates_[OrthoNodeData::Y_AXIS].end());
    sort(coordinates_[OrthoNodeData::Z_AXIS].begin(), coordinates_[OrthoNodeData::Z_AXIS].end());

    int numShields = shields.size();

    //create _celValueAryShield
    int cellValueArySize = (coordinates_[OrthoNodeData::X_AXIS].size() - 1)*(coordinates_[OrthoNodeData::Y_AXIS].size() - 1)*(coordinates_[OrthoNodeData::Z_AXIS].size() - 1);
    cell_shield_values_ = new vector<double>[numShields] {};

    //yamFlag Array

    int iShield = 0;
    for(auto& item : shields) {
        cell_shield_values_[iShield].clear();
        cell_shield_values_[iShield].resize(cellValueArySize);

        string shieldMaterial = get<0>(item);
        double shieldThickness = get<1>(item);

        int id = 0;
        bool yamlFlg = false;
        for(int i = 0; i < nMat; i++) {
            if(shieldMaterial == shieldTable.sMat[i]) {
                id = i;
                density = shieldTable.sDensity[i];
                yamlFlg = true;
                break;
            }
        }

        for(int i = 0; i < nEne; i++) {
            mAEne[i] = shieldTable.mAEne[id][i];
            mAVal[i] = shieldTable.mAVal[id][i];
            dTEne[i] = shieldTable.dTEne[id][i];

            for(int j = 0; j < nMFP; j++) {
                dTMFP[j] = shieldTable.dTMFP[id][j];
                dTVal[i][j] = shieldTable.dTVal[id][i][j];
            }
        }

        int xNum = 0;
        int yNum = 0;
        int zNum = 0;
        for(int i = 0; i < directionNumber; i++) {
            //cout<<i<<endl;
            GammaData::CalcDirectionRecInfo recInfo = dataInfo.calcDirectionRec[i];
            xNum = yNum = zNum = 0;
            calcPoint[0] = origin[0] + recInfo.directionX;
            calcPoint[1] = origin[1] + recInfo.directionY;
            calcPoint[2] = origin[2] + recInfo.directionZ;

            for(int j = 0; j < coordinates_[OrthoNodeData::X_AXIS].size() - 1; j++) {
                if(fabs((coordinates_[OrthoNodeData::X_AXIS][j] + coordinates_[OrthoNodeData::X_AXIS][j + 1])*0.5 - calcPoint[0]) < arrowableError_) {
                    xNum = j;
                    break;
                }
            }

            for(int j = 0; j < coordinates_[OrthoNodeData::Y_AXIS].size() - 1; j++) {
                if(fabs((coordinates_[OrthoNodeData::Y_AXIS][j] + coordinates_[OrthoNodeData::Y_AXIS][j + 1])*0.5 - calcPoint[1]) < arrowableError_) {
                    yNum = j;
                    break;
                }
            }

            for(int j = 0; j < coordinates_[OrthoNodeData::Z_AXIS].size() - 1; j++) {
                if(fabs((coordinates_[OrthoNodeData::Z_AXIS][j] + coordinates_[OrthoNodeData::Z_AXIS][j + 1])*0.5 - calcPoint[2]) < arrowableError_) {
                    zNum = j;
                    break;
                }
            }
            for(int j = 0; j < gammaData.energySpectrumChannelNumber(); ++j) {

                float energy = (gammaData.energySpectrumMax() - gammaData.energySpectrumMin()) / gammaData.energySpectrumChannelNumber() * (j + 1);

                bool interPE = false;
                int ie = 0;
                if(energy > dTEne[nEne - 1]) {
                    ie = nEne - 1;
                    interPE = true;
                } else {
                    for(int i = 0; i < nEne; i++) {
                        if(energy == dTEne[i]) {
                            ie = i;
                            break;
                        } else if(energy < dTEne[i]) {
                            ie = i;
                            interPE = true;
                            break;
                        }
                    }
                }

                double ramda = 0.0;
                if(interPE == false) {
                    ramda = mAVal[ie];
                } else if(interPE) {
                    if(ie == 0) {
                        ramda = interPolation(energy, mAEne[ie + 1], mAEne[ie], mAVal[ie + 1], mAVal[ie]);
                    } else {
                        ramda = interPolation(energy, mAEne[ie], mAEne[ie - 1], mAVal[ie], mAVal[ie - 1]);
                    }
                }

                MFP = ramda * density * shieldThickness;

                bool interPM = false;
                int imfp = 0;
                if(MFP > dTMFP[nMFP - 1]) {
                    imfp = nMFP - 1;
                    interPM = true;
                } else {
                    for(int i = 0; i < nMFP; i++) {
                        if(MFP == dTMFP[i]) {
                            imfp = i;
                            break;
                        } else if(MFP < dTMFP[i]) {
                            imfp = i;
                            interPM = true;
                            break;
                        }
                    }
                }

                double val = 0.0;
                if(interPE == false && interPM == false) {
                    val = dTVal[ie][imfp];
                } else if(interPE && interPM == false) {
                    if(ie == 0) {
                        val = interPolation(energy, dTEne[ie + 1], dTEne[ie], dTVal[ie + 1][imfp], dTVal[ie][imfp]);
                    } else {
                        val = interPolation(energy, dTEne[ie], dTEne[ie - 1], dTVal[ie][imfp], dTVal[ie - 1][imfp]);
                    }
                } else if(interPE == false && interPM) {
                    if(imfp == nMFP - 1) {
                        val = dTVal[ie][imfp];
                    } else {
                        val = interPolation(MFP, dTMFP[imfp], dTMFP[imfp - 1], dTVal[ie][imfp], dTVal[ie][imfp - 1]);
                    }
                } else if(interPE && interPM) {
                    double val1 = 0.0;
                    double val2 = 0.0;

                    if(ie == 0) {
                        val2 = interPolation(energy, dTEne[ie + 1], dTEne[ie], dTVal[ie + 1][imfp], dTVal[ie][imfp]);
                        val1 = interPolation(energy, dTEne[ie + 1], dTEne[ie], dTVal[ie + 1][imfp - 1], dTVal[ie][imfp - 1]);
                    } else {
                        val2 = interPolation(energy, dTEne[ie], dTEne[ie - 1], dTVal[ie][imfp], dTVal[ie - 1][imfp]);
                        val1 = interPolation(energy, dTEne[ie], dTEne[ie - 1], dTVal[ie][imfp - 1], dTVal[ie - 1][imfp - 1]);

                    }

                    if(imfp == nMFP - 1) {
                        val = dTVal[ie][imfp];
                    } else {
                        val = interPolation(MFP, dTMFP[imfp], dTMFP[imfp - 1], val2, val1);
                    }
                }
                if(yamlFlg) {
                    cell_shield_values_[iShield][(coordinates_[OrthoNodeData::Y_AXIS].size() - 1)*(coordinates_[OrthoNodeData::Z_AXIS].size() - 1)*xNum + (coordinates_[OrthoNodeData::Z_AXIS].size() - 1)*yNum + zNum] += (recInfo.dirData[j] * gammaData.scaleFactor()*dataInfo.scaleFactor) * val;
                } else {
                    cell_shield_values_[iShield][(coordinates_[OrthoNodeData::Y_AXIS].size() - 1)*(coordinates_[OrthoNodeData::Z_AXIS].size() - 1)*xNum + (coordinates_[OrthoNodeData::Z_AXIS].size() - 1)*yNum + zNum] += 0.0;
                }
            }
        }
        iShield++;
    }
    isValid_ = true;
    shieldTable.clear();
    return true;
}


OrthoNodeData::OrthoNodeData(const GammaData& gammaData)
{
    OrthoCellData cellGrid;
    cellGrid.createSampleData(gammaData);
    if(!cellGrid.isValid()) {
        return;
    }

    clear();

    cell_.resize(0);
    size_t xCellSize = cellGrid.size(AxisID::X_AXIS);
    size_t yCellSize = cellGrid.size(AxisID::Y_AXIS);
    size_t zCellSize = cellGrid.size(AxisID::Z_AXIS);
    cell_.resize(xCellSize, yCellSize, zCellSize);
    for(size_t z = 0 ; z < zCellSize ; ++z) {
        for(size_t y = 0 ; y < yCellSize ; ++y) {
            for(size_t x = 0 ; x < xCellSize ; ++x) {
                cell_(x, y, z) = cellGrid.value(x, y, z);
            }
        }
    }

    coordinates_[X_AXIS] = cellGrid.coordinates(AxisID::X_AXIS);
    coordinates_[Y_AXIS] = cellGrid.coordinates(AxisID::Y_AXIS);
    coordinates_[Z_AXIS] = cellGrid.coordinates(AxisID::Z_AXIS);

    min_ = cellGrid.min();
    max_ = cellGrid.max();

    size_t xNodeSize = xCellSize + 1;
    size_t yNodeSize = yCellSize + 1;
    size_t zNodeSize = zCellSize + 1;

    array3d xNodeGrid;
    xNodeGrid.resize(xNodeSize, yCellSize, zCellSize);

    for(size_t k = 0 ; k < zCellSize ; ++k) {
        for(size_t j = 0 ; j < yCellSize ; ++j) {
            for(size_t i = 0 ; i < xNodeSize ; ++i) {
                if(i == 0) {
                    xNodeGrid(i, j, k) = cell_(i, j, k);
                } else if(i == xCellSize) {
                    xNodeGrid(i, j, k) = cell_(i - 1, j, k);
                } else {
                    Boxd pb = cellBounds(i - 1, j, k);
                    Boxd nb = cellBounds(i, j, k);
                    xNodeGrid(i, j, k) = linearInterpolateByLength(cell_(i - 1, j, k), cell_(i, j, k), pb.x() / 2.0, nb.x() / 2.0);

                }
            }
        }
    }

    array3d yNodeGrid;
    yNodeGrid.resize(xNodeSize, yNodeSize, zCellSize);
    for(size_t k = 0 ; k < zCellSize ; ++k) {
        for(size_t j = 0 ; j < yNodeSize ; j++) {
            for(size_t i = 0 ; i < xNodeSize ; i++) {
                if(j == 0) {
                    yNodeGrid(i, j, k) = xNodeGrid(i, j, k);
                } else if(j == yCellSize) {
                    yNodeGrid(i, j, k) = xNodeGrid(i, j - 1, k);
                } else {
                    Boxd pb = cellBounds(0, j - 1, 0);
                    Boxd nb = cellBounds(0, j - 1, 0);
                    double pv = xNodeGrid(i, j - 1, k);
                    double nv = xNodeGrid(i, j, k);
                    yNodeGrid(i, j, k) = linearInterpolateByLength(pv, nv, pb.y() / 2.0, nb.y() / 2.0);
                }
            }
        }
    }
    xNodeGrid.resize(0);

    node_.resize(0);
    node_.resize(xNodeSize, yNodeSize, zNodeSize);

    for(size_t k = 0 ; k < zNodeSize ; ++k) {
        for(size_t j = 0 ; j < yNodeSize ; j++) {
            for(size_t i = 0 ; i < xNodeSize ; i++) {
                if(k == 0) {
                    node_(i, j, k) = yNodeGrid(i, j, k);
                } else if(k == zCellSize) {
                    node_(i, j, k) = yNodeGrid(i, j, k - 1);
                } else {
                    Boxd pb = cellBounds(0, 0, k - 1);
                    Boxd nb = cellBounds(0, 0, k);
                    double pv = yNodeGrid(i, j, k - 1);
                    double nv = yNodeGrid(i, j, k);
                    node_(i, j, k) = linearInterpolateByLength(pv, nv, pb.z() / 2.0, nb.z() / 2.0);
                }
            }
        }
    }
    isValid_ = true;
}


OrthoNodeData::~OrthoNodeData()
{

}


bool OrthoNodeData::createShieldData(string& filename, const GammaData& gammaData)
{
    OrthoCellData cellGrid;
    cellGrid.createShieldData(filename, gammaData);
    if(!cellGrid.isValid()) {
        return false;
    }

    int numShield = shields.size();

    cell_shield_ = new array3d[numShield] {};

    size_t xsize = cellGrid.size(AxisID::X_AXIS);
    size_t ysize = cellGrid.size(AxisID::Y_AXIS);
    size_t zsize = cellGrid.size(AxisID::Z_AXIS);
    for(int i = 0; i < shields.size(); ++i) {
        cell_shield_[i].resize(0);
        cell_shield_[i].resize(xsize, ysize, zsize);
        for(size_t z = 0; z < zsize; ++z) {
            for(size_t y = 0; y < ysize; ++y) {
                for(size_t x = 0; x < xsize; ++x) {
                    cell_shield_[i](x, y, z) = cellGrid.value_shield(i, x, y, z);
                }
            }
        }
    }
    return true;
}


void OrthoNodeData::addShield(const string& material, const double& thickness)
{
    shields.push_back(make_tuple(material, thickness));
}


void OrthoNodeData::clearShield()
{
    shields.clear();
}


void OrthoNodeData::clear()
{
    cell_.resize(0);
    node_.resize(0);
    coordinates_[X_AXIS].clear();
    coordinates_[Y_AXIS].clear();
    coordinates_[Z_AXIS].clear();
    isValid_ = false;
}


double OrthoNodeData::value(const Vector3d& pos) const
{
    uint32_t i,j,k;
    if(findCellIndex(pos, i, j, k) == false) {
        return numeric_limits<double>::quiet_NaN();
    }

    array<double, 8> nodeValAry;
    nodeValAry[0] = node_(i,j,k);
    nodeValAry[1] = node_(i+1,j,k);
    nodeValAry[2] = node_(i+1,j+1,k);
    nodeValAry[3] = node_(i,j+1,k);

    nodeValAry[4] = node_(i,j,k+1);
    nodeValAry[5] = node_(i+1,j,k+1);
    nodeValAry[6] = node_(i+1,j+1,k+1);
    nodeValAry[7] = node_(i,j+1,k+1);

    Boxd bounds = cellBounds(i,j,k);

    Vector3d min = bounds.min();
    Vector3d max = bounds.max();

    array<double, 4> xValAry;
    xValAry[0] = linearInterpolateByPosition(nodeValAry[0], nodeValAry[1], pos.x(), min.x(), max.x());
    xValAry[1] = linearInterpolateByPosition(nodeValAry[3], nodeValAry[2], pos.x(), min.x(), max.x());
    xValAry[2] = linearInterpolateByPosition(nodeValAry[7], nodeValAry[6], pos.x(), min.x(), max.x());
    xValAry[3] = linearInterpolateByPosition(nodeValAry[4], nodeValAry[5], pos.x(), min.x(), max.x());

    array<double, 2> yValAry;
    yValAry[0] = linearInterpolateByPosition(xValAry[0], xValAry[1], pos.y(), min.y(), max.y());
    yValAry[1] = linearInterpolateByPosition(xValAry[3], xValAry[2], pos.y(), min.y(), max.y());

    double zVal = linearInterpolateByPosition(yValAry[0], yValAry[1], pos.z(), min.z(), max.z());

    return zVal;
}


bool OrthoNodeData::findCellIndex(const Vector3d& pos, uint32_t& i, uint32_t& j, uint32_t& k) const
{
    size_t xCellSize = coordinates_[X_AXIS].size()-1;
    size_t yCellSize = coordinates_[Y_AXIS].size()-1;
    size_t zCellSize = coordinates_[Z_AXIS].size()-1;
    bool found = false;
    for(size_t p=0 ; p<xCellSize ; ++p) {
        if(coordinates_[X_AXIS][p] <= pos.x() && pos.x() <= coordinates_[X_AXIS][p+1]) {
            i = static_cast<uint32_t>(p);
            found = true;
            break;
        }
    }
    if(found == false) {
        return false;
    }

    found = false;
    for(size_t p=0 ; p<yCellSize ; ++p) {
        if(coordinates_[Y_AXIS][p] <= pos.y() && pos.y() <= coordinates_[Y_AXIS][p+1]) {
            j = static_cast<uint32_t>(p);
            found = true;
            break;
        }
    }
    if(found == false) {
        return false;
    }

    found = false;
    for(size_t p=0 ; p<zCellSize ; ++p) {
        if(coordinates_[Z_AXIS][p] <= pos.z() && pos.z() <= coordinates_[Z_AXIS][p+1]) {
            k = static_cast<uint32_t>(p);
            found = true;
            break;
        }
    }
    if(found == false) {
        return false;
    }

    return true;
}


ShieldTable::ShieldTable()
{
    nMat = 0;
    nEne = 25;
    nMFP = 17;
}


ShieldTable::~ShieldTable()
{

}


bool ShieldTable::load(const string& filename, ostream& os)
{
    //    sMat[nMat] = {};
    //    sDensity[nMat] = {};
    //    mAEne[nMat][nEne] = {};
    //    mAVal[nMat][nEne] = {};
    //    dTEne[nMat][nEne] = {};
    //    dTMFP[nMat][nMFP] = {};
    //    dTVal[nMat][nEne][nMFP] = {};

    try {
       YAMLReader reader;
       MappingPtr node = reader.loadDocument(filename)->toMapping();
       if(node) {
          auto& shieldList = *node->findListing("shields");
          if(shieldList.isValid()) {
             nMat = shieldList.size();

             sMat = new string [nMat] {};
             sDensity = new double [nMat] {};
             mAEne = new double *[nMat] {};
             mAVal = new double *[nMat] {};
             dTEne = new double *[nMat] {};
             dTMFP = new double *[nMat] {};
             dTVal = new double **[nMat] {};

             for(int i = 0; i < nMat; ++i) {
                mAEne[i] = new double[nEne] {};
                mAVal[i] = new double[nEne] {};
                dTEne[i] = new double[nEne] {};
                dTMFP[i] = new double[nMFP] {};
                dTVal[i] = new double*[nEne] {};

                for(int j = 0; j < nEne; ++j) {
                    dTVal[i][j] = new double[nMFP] {};
                }
             }

             for(int i = 0; i < shieldList.size(); ++i) {
                Mapping* info = shieldList[i].toMapping();
                string name;
                info->read("name", name);
                sMat[i] = name;

                double density;
                info->read("density", density);
                sDensity[i] = density;

                auto& aenergyList = *info->findListing("massAttenuationEnergy");
                if(aenergyList.size() != nEne) {
                    ValueNode::SyntaxException ex;
                    ex.setPosition(ex.line(), ex.column());
                    ex.setMessage(_("The list is incomplete"));
                    throw ex;
                }
                for(int j = 0; j < aenergyList.size(); ++j) {
                    mAEne[i][j] = aenergyList[j].toDouble();
                }

                auto& avalueList = *info->findListing("massAttenuationValue");
                if(avalueList.size() != nEne) {
                    ValueNode::SyntaxException ex;
                    ex.setPosition(ex.line(), ex.column());
                    ex.setMessage(_("The list is incomplete"));
                    throw ex;
                }
                for(int j = 0; j < avalueList.size(); ++j) {
                    mAVal[i][j] = avalueList[j].toDouble();
                }

                auto& tenergyList = *info->findListing("doseTransmittanceEnergy");
                if(tenergyList.size() != nEne) {
                    ValueNode::SyntaxException ex;
                    ex.setPosition(ex.line(), ex.column());
                    ex.setMessage(_("The list is incomplete"));
                    throw ex;
                }
                for(int j = 0; j < tenergyList.size(); ++j) {
                    dTEne[i][j] = tenergyList[j].toDouble();
                }

                auto& mfpList = *info->findListing("doseTransmittanceMFP");
                if(mfpList.size() != nMFP) {
                    ValueNode::SyntaxException ex;
                    ex.setPosition(ex.line(), ex.column());
                    ex.setMessage(_("The list is incomplete"));
                    throw ex;
                }
                for(int j = 0; j < mfpList.size(); j++) {
                    dTMFP[i][j] = mfpList[j].toDouble();
                }

                double size = mfpList.size() *  tenergyList.size();
                auto& tvalueList = *info->findListing("doseTransmittanceValue");
                if(tvalueList.size() != size) {
                    ValueNode::SyntaxException ex;
                    ex.setPosition(ex.line(), ex.column());
                    ex.setMessage(_("The list is incomplete"));
                    throw ex;
                }
                for(int j = 0; j < tenergyList.size(); ++j) {
                    for(int k = 0; k < mfpList.size(); ++k) {
                       int id = (tenergyList.size() * k) + j;
                       dTVal[i][j][k] = tvalueList[id].toDouble();
                    }
                }
             }
          }
       }
    }
    catch (const ValueNode::Exception& ex) {
        os << ex.message();
    }
    return true;
}


void ShieldTable::clear()
{
    delete[] sMat;
    delete[] sDensity;
    delete[] mAEne;
    delete[] mAVal;
    delete[] dTEne;
    delete[] dTMFP;
    delete[] dTVal;
}
