/**
   @author Kenta Suzuki
*/

#ifndef CNOID_PHITS_PLUGIN_GAMMA_DATA_H
#define CNOID_PHITS_PLUGIN_GAMMA_DATA_H

#include <cnoid/EigenUtil>
#include <string>
#include <vector>

namespace cnoid {

class GammaData
{
public:
    GammaData();
    virtual ~GammaData() { }

    enum DataType { DOSERATE, PINHOLE, COMPTON };

    struct GeometryInfo {
        int calcPointID;
        std::vector<float> calcPoint;//size:3
        int calcDirectionNumber;
    };

    struct CalcDirectionPoInfo {
        int directionID;
        float phi;
        float lambda;
        float distance;
        float deltaPhi;
        float deltaLambda;
        float deltaDistance;
        std::vector<float> dirData;
    };

    struct CalcDirectionRecInfo {
        int directionID;
        float directionX;
        float directionY;
        float directionZ;
        float deltaX;
        float deltaY;
        float deltaZ;
        std::vector<float> dirData;
    };

    struct DataInfo {
        int calcPointID;
        std::vector<float> calcPoint;//size:3
        float scaleFactor;
        int calcDirectionNumber;
        std::vector<CalcDirectionPoInfo> calcDirectionPo;
        std::vector<CalcDirectionRecInfo> calcDirectionRec;
    };

    struct CalcInfo {

        struct ElementInfo {
            int n = 1;
            double min = 0.0;
            double max = 10.0;
        };

        int maxcas = 10000;
        int maxbch = 10;
        ElementInfo xyze[4];
        int inputMode = DOSERATE;
        int nSrc = 0;
        Matrix3 srcRotMat[100];
    };

    bool setDataHeaderInfo(GeometryInfo geoinfo);
    bool getDataHeaderInfo(GeometryInfo geoinfo);
    int dataMode() const { return _dataMode; }
    int timeUnit() const { return timeUnit_; }
    int energySpectrumChannelNumber() const { return _energySpectrumChannelNumber; }
    float energySpectrumMax() const { return _energySpectrumMax; }
    float energySpectrumMin() const { return _energySpectrumMin; }
    float scaleFactor() const { return _scaleFactor; }
    int getCalculatingPointNumber() const { return _calculatingPointNumber; }
    GeometryInfo geometryInfo(const int& number) const { return _geometryHeaderInfo[number]; }

    void addDataInfo(const DataInfo& dataInfo);
    DataInfo dataInfo() const { return _dataInfo; }

    bool read(const std::string& filename);
    bool write(const std::string& filename);
    bool readPHITS(const std::string& filename, const uint8_t _readMode);
    bool readQAD(const std::string& filename, CalcInfo calcInfo, int iSrc);

private:
    std::string filename_;
    int _dataMode;
    std::string comment_;
    std::string unitName_;
    int timeUnit_;
    std::vector<float> origin_;
    int _energySpectrumChannelNumber;
    float _energySpectrumMax;
    float _energySpectrumMin;
    float _scaleFactor;
    int _calculatingPointNumber;
    std::vector<GeometryInfo> _geometryHeaderInfo;
    DataInfo _dataInfo;

    std::string title;
    float xmin, ymin, zmin, emin;
    float xmax, ymax, zmax, emax;
    float delX, delY, delZ, delE;
    int nx, ny, nz, ne;
};

}

#endif // CNOID_PHITS_PLUGIN_GAMMA_DATA_H
