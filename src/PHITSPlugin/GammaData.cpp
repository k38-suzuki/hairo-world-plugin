/**
   @author Kenta Suzuki
*/

#include "GammaData.h"
#include <cnoid/EigenUtil>
#include <fmt/format.h>
#include <fstream>
#include <sstream>
#include <math.h>
#include <iostream>
#include "gettext.h"

using namespace std;
using namespace cnoid;

namespace {

struct PHITSDataInfo {
    float xdata;
    float ydata;
    float zdata;
    vector<float> ddata;
};

string getLineN(string str, int n)
{
    vector<string> v;
    string s;
    stringstream ss;

    ss << str;

    while(getline(ss, s, ' ')) {
        if(s != "" && s != "#")  v.push_back(s);
    }
    //for(const string& s : v) {         // vの中身を出力
    //    cout << s << endl;
    //}

    if(v.size() > n) {
        return v[n];
    } else {
        return "";
    }
}

string getLineRight(string str)
{
    vector<string> v, vout;
    string s;
    stringstream ss;

    ss << str;

    while(getline(ss, s, '=')) {
        if(s != "")  v.push_back(s);
    }
    //for(const string& s : v) {         // vの中身を出力
    //    cout << s << endl;
    //}

    ss.clear();
    ss << v[1];
    while(getline(ss, s, '#')) {
        if(s != "")  vout.push_back(s);
    }

    return vout[0];
}

}


GammaData::GammaData()
{

}


bool GammaData::read(const string& filename)
{
    ifstream in;
    in.open(filename.data(),ios_base::in |ios_base::binary);
    if(!in) {
//        cout << "Binary file was not found." << endl;
        return false;
    }
    filename_ = filename;

    //file header
    in.seekg(0,ios_base::beg);
    in.read((char *) &_dataMode,4);
    char dammy[1024]="";
    in.read((char *) dammy,1024);
    comment_=string(dammy);
    char dammy2[8]="";
    in.read((char *) dammy2,8);
    unitName_=string(dammy2);
    in.read((char *) &timeUnit_,4);
    comment_=string(dammy);
    vector<float> origin;
    origin.resize(3);
    in.read((char *) &origin[0],4);
    in.read((char *) &origin[1],4);
    in.read((char *) &origin[2],4);
    origin_=origin;

    in.read((char *)&_energySpectrumChannelNumber,4);
    in.read((char *)&_energySpectrumMax,4);
    in.read((char *)&_energySpectrumMin,4);
    in.read((char *)&_scaleFactor,4);
    int calculatingPointNumber;
    in.read((char *)&calculatingPointNumber,4);
    _calculatingPointNumber=calculatingPointNumber;

    vector<GeometryInfo> geometryHeaderInfo;
    geometryHeaderInfo.resize(calculatingPointNumber);

    //geometry header
    in.seekg(2048,ios_base::beg);
    for(int i = 0;i<calculatingPointNumber;i++) {
        GeometryInfo geometryHeader;
        origin.resize(3);
        in.read((char *) &geometryHeader.calcPointID,4);
        in.read((char *) &origin[0],4);
        in.read((char *) &origin[1],4);
        in.read((char *) &origin[2],4);
        geometryHeader.calcPoint=origin;
        in.read((char *) &geometryHeader.calcDirectionNumber,4);
        geometryHeaderInfo[i]=geometryHeader;
        in.seekg(12,ios_base::cur);
    }
    _geometryHeaderInfo=geometryHeaderInfo;
    in.close();

    return true;
}


bool GammaData::readPHITS(const string& filename, const uint8_t _readMode)
{    
    // phits outputからデータの読み込み
    ifstream in;
    in.open(filename, ios_base::in);
    if(!in) {
        cout << "Output file was not found." << endl;
        return false;
    }
    filename_ = filename.data();

    string str;
    string buf, buf2;

    do {
        getline(in, str);

        if(in.eof()) {
            cout << "The file reached the end while reading." << endl;
            in.close();
            return false;
        }

        buf = getLineN(str, 0);
        buf2 = getLineN(str, 1);

        // title
        if(buf == "title" && buf2 == "=") {
            str = getLineRight(str);
            title = str;
        }

        // xmin
        if(buf == "xmin" && buf2 == "=") {
            str = getLineRight(str);
            stringstream ss{ str };
            ss >> xmin;
            xmin /= 100; // [cm]->[m]
        }
        // xmax
        if(buf == "xmax" && buf2 == "=") {
            str = getLineRight(str);
            stringstream ss{ str };
            ss >> xmax;
            xmax /= 100; // [cm]->[m]
        }
        // nx
        if(buf == "nx" && buf2 == "=") {
            str = getLineRight(str);
            stringstream ss{ str };
            ss >> nx;
        }

        // ymin
        if(buf == "ymin" && buf2 == "=") {
            str = getLineRight(str);
            stringstream ss{ str };
            ss >> ymin;
            ymin /= 100; // [cm]->[m]
        }
        // ymax
        if(buf == "ymax" && buf2 == "=") {
            str = getLineRight(str);
            stringstream ss{ str };
            ss >> ymax;
            ymax /= 100; // [cm]->[m]
        }
        // ny
        if(buf == "ny" && buf2 == "=") {
            str = getLineRight(str);
            stringstream ss{ str };
            ss >> ny;
        }

        // zmin
        if(buf == "zmin" && buf2 == "=") {
            str = getLineRight(str);
            stringstream ss{ str };
            ss >> zmin;
            zmin /= 100; // [cm]->[m]
        }
        // zmax
        if(buf == "zmax" && buf2 == "=") {
            str = getLineRight(str);
            stringstream ss{ str };
            ss >> zmax;
            zmax /= 100; // [cm]->[m]
        }
        // nz
        if(buf == "nz" && buf2 == "=") {
            str = getLineRight(str);
            stringstream ss{ str };
            ss >> nz;
        }

        // emin
        if(buf == "emin" && buf2 == "=") {
            str = getLineRight(str);
            stringstream ss{ str };
            ss >> emin;
        }
        // emax
        if(buf == "emax" && buf2 == "=") {
            str = getLineRight(str);
            stringstream ss{ str };
            ss >> emax;
        }
        // ne
        if(buf == "ne" && buf2 == "=") {
            str = getLineRight(str);
            stringstream ss{ str };
            ss >> ne;
            break;
        }


    } while(!in.eof());

    delX = (xmax - xmin) / nx;
    delY = (ymax - ymin) / ny;
    delZ = (zmax - zmin) / nz;
    vector<PHITSDataInfo> phitsData;
    if(_readMode == DOSERATE || _readMode == COMPTON) {
        for(int i = 0; i < nz; ++i) {
            float z = delZ * i + zmin + delZ / 2.0;
            for(int j = 0; j < ny; ++j) {
                float y = -delY * j + ymax - delY / 2.0;
                for(int k = 0; k < nx; ++k) {
                    PHITSDataInfo info;
                    float x = delX * k + xmin + delX / 2.0;
                    info.xdata = x;
                    info.ydata = y;
                    info.zdata = z;
                    phitsData.push_back(info);
                }
            }
        }
    } else if(_readMode == PINHOLE) {
        for(int k = 0; k < nx; ++k) {
            float x = delX * k + xmin + delX / 2.0;
            for(int j = 0; j < ny; ++j) {
                float y = -delY * j + ymax - delY / 2.0;
                for(int i = 0; i < nz; ++i) {
                    PHITSDataInfo info;
                    float z = delZ * i + zmin + delZ / 2.0;
                    info.xdata = x;
                    info.ydata = y;
                    info.zdata = z;
                    phitsData.push_back(info);
                }
            }
        }
    }

    int index_data = 0;
    int ie = 0;
    int iz = 0;
    do {
        getline(in, str);
        buf = getLineN(str, 0);
        buf2 = getLineN(str, 1);

        // hc:
        if(buf == "hc:" && buf2 == "y") {
            int maxi;
            if(_readMode == DOSERATE) {
                maxi = nx * ny / 10;
            } else if(_readMode == PINHOLE || _readMode == COMPTON) {
                maxi = nx * ny * nz / 10;
            }

            if(fmod(nx*ny, 10) != 0) maxi += 1;
            for(int line = 0; line < maxi; ++line) {
                getline(in, str);
                stringstream ss{ str };
                vector<string> v;
                string s;

                while(getline(ss, s, ' ')) {
                    if(s != "")  v.push_back(s);
                }
                
                for(const string& sv : v) {
                    stringstream sss{ sv };
                    float d;
                    sss >> d;
                    phitsData[index_data].ddata.push_back(d);
                    index_data += 1;
                }
            }
            if(ie < ne - 1) {
                ie += 1;
                index_data = iz * nx * ny;
            } else {
                ie = 0;
                iz += 1;
            }
        }
        //if(index_data >= nx * ny * nz * ne) break;

    } while(!in.eof());

    in.close();

    // DoseSlice用のgammaDataFileの作成

    //file header
    _dataMode = 1;
    char dammy[1024] = "";
    title.copy(dammy, 1024);
    comment_ = string(dammy);
    char dammy2[8] = "μSv/h";
    unitName_ = string(dammy2);
    timeUnit_ = 3600;

    vector<float> origin;
    origin.resize(3);
    origin[0] = 0.0;
    origin[1] = 0.0;
    origin[2] = 0.0;
    origin_ = origin;

    _energySpectrumChannelNumber = ne;
    _energySpectrumMax = emax;
    _energySpectrumMin = emin;
    _scaleFactor = 1.0;
    _calculatingPointNumber = 1;

    vector<GeometryInfo> geometryHeaderInfo;
    geometryHeaderInfo.resize(_calculatingPointNumber);

    //geometry header
    for(int i = 0; i < _calculatingPointNumber; i++) {
        GeometryInfo geometryHeader;
        geometryHeader.calcPointID = i + 1;
        geometryHeader.calcPoint = origin;
        geometryHeader.calcDirectionNumber = nx * ny * nz;
        geometryHeaderInfo[i] = geometryHeader;
    }
    _geometryHeaderInfo = geometryHeaderInfo;

    //data header
    GeometryInfo dammyGI;
    float scaleFactor;
    dammyGI.calcPoint.resize(3);
    dammyGI.calcPointID = _geometryHeaderInfo[0].calcPointID;
    dammyGI.calcPoint = _geometryHeaderInfo[0].calcPoint;
    scaleFactor = 1.0;
    dammyGI.calcDirectionNumber = _geometryHeaderInfo[0].calcDirectionNumber;

    GeometryInfo geoInfo = this->geometryInfo(0);

    if(geoInfo.calcPointID != dammyGI.calcPointID || geoInfo.calcPoint != dammyGI.calcPoint || geoInfo.calcDirectionNumber != dammyGI.calcDirectionNumber) {
        return false;
    }

    _dataInfo.calcPointID = geoInfo.calcPointID;
    _dataInfo.calcPoint = geoInfo.calcPoint;
    _dataInfo.scaleFactor = scaleFactor;
    _dataInfo.calcDirectionNumber = geoInfo.calcDirectionNumber;
    if(_dataMode == 1) {
        _dataInfo.calcDirectionRec.resize(_dataInfo.calcDirectionNumber);
        for(int i = 0; i < _dataInfo.calcDirectionNumber; i++) {
            _dataInfo.calcDirectionRec[i].directionID = i + 1;
            _dataInfo.calcDirectionRec[i].directionX = phitsData[i].xdata;
            _dataInfo.calcDirectionRec[i].directionY = phitsData[i].ydata;
            _dataInfo.calcDirectionRec[i].directionZ = phitsData[i].zdata;
            _dataInfo.calcDirectionRec[i].deltaX = delX;
            _dataInfo.calcDirectionRec[i].deltaY = delY;
            _dataInfo.calcDirectionRec[i].deltaZ = delZ;
            _dataInfo.calcDirectionRec[i].dirData.resize(_energySpectrumChannelNumber);
            for(int j = 0; j < _energySpectrumChannelNumber; j++) {
                _dataInfo.calcDirectionRec[i].dirData[j] = phitsData[i].ddata[j];
            }
        }
    } else {
        return false;
    }

    return true;
}


bool GammaData::readQAD(const string& filename, CalcInfo calcInfo, int iSrc)
{
    // QAD outputからデータの読み込み
    ifstream in;
    in.open(filename, ios_base::in);

    if(!in) {
        cout << "Output file was not found." << endl;
        return false;
    }
    filename_ = filename.data();

    string str;
    string buf, buf2;
    string bufX, bufY, bufZ, bufG;

    double trX = 0.0;
    double trY = 0.0;
    double trZ = 0.0;

    vector<PHITSDataInfo> phitsData;

    do {
        getline(in, str);

        buf = getLineN(str, 0);
        buf2 = getLineN(str, 1);

        // title、energ、TR
        if(buf == "1" && buf2 == "ne=") {
            // title
            title = str;

            // ne
            buf = getLineN(str, 2);
            ne = stoi(buf);

            // emin
            buf = getLineN(str, 4);
            emin = stod(buf);

            // emax
            buf = getLineN(str, 6);
            emax = stod(buf);

            // trX
            buf = getLineN(str, 9);
            trX = stod(buf);

            // trY
            buf = getLineN(str, 11);
            trY = stod(buf);;

            // trZ
            buf = getLineN(str, 13);
            trZ = stod(buf);
        }

        // 評価点
        if(buf2 == "PHOTONS") {

            do {
                getline(in, str);

                int  geoOption = 0;
                float x = 0;
                float y = 0;
                float z = 0;

                bufX = getLineN(str, 1);
                x = stod(bufX);

                bufZ = getLineN(str, 2);
                z = stod(bufZ);

                bufY = getLineN(str, 3);
                y = stod(bufY);

                bufG = getLineN(str, 4);
                geoOption = stoi(bufG);
                if(geoOption == -1) break;

                // 先にx,y,zを線源の姿勢行列で回転させる
                Vector3 v = { x,y,z };
                v = calcInfo.srcRotMat[iSrc] * v;

                x = v[0] + trX;
                y = v[1] + trY;
                z = v[2] + trZ;
                x /= 100;    // [cm]->[m]
                y /= 100;    // [cm]->[m]
                z /= 100;    // [cm]->[m]

                PHITSDataInfo info;
                info.xdata = x;
                info.ydata = y;
                info.zdata = z;
                phitsData.push_back(info);

                if(in.eof()) {
                    in.close();
                    return false;
                }
            } while(!in.eof());

            break;
        }

        if(in.eof()) {
            //cout << "ファイルが終端に達しました。" << endl;
            in.close();
            return false;
        }

    } while(!in.eof());

    nx = calcInfo.xyze[0].n;
    ny = calcInfo.xyze[1].n;
    nz = calcInfo.xyze[2].n;
    delX = (calcInfo.xyze[0].max - calcInfo.xyze[0].min) / calcInfo.xyze[0].n;
    delY = (calcInfo.xyze[1].max - calcInfo.xyze[1].min) / calcInfo.xyze[1].n;
    delZ = (calcInfo.xyze[2].max - calcInfo.xyze[2].min) / calcInfo.xyze[2].n;

    int index_data = 0;
    do {
        getline(in, str);
        buf = getLineN(str, 0);

        // total:
        if(buf == "TOTAL") {
            buf = getLineN(str, 9);
            stringstream ss{ buf };
            float d;
            ss >> d;
            phitsData[index_data].ddata.push_back(d);
            index_data += 1;
        }
        if(index_data >= nx * ny * nz) break;

    } while(!in.eof());

    in.close();

    // DoseSlice用のgammaDataFileの作成

    //file header
    _dataMode = 1;
    char dammy[1024] = "";
    title.copy(dammy, 1024);
    comment_ = string(dammy);
    char dammy2[8] = "μSv/h";
    unitName_ = string(dammy2);
    timeUnit_ = 3600;

    vector<float> origin;
    origin.resize(3);
    origin[0] = 0.0;
    origin[1] = 0.0;
    origin[2] = 0.0;
    origin_ = origin;

    _energySpectrumChannelNumber = ne;
    _energySpectrumMax = emax;
    _energySpectrumMin = emin;
    _scaleFactor = 1.0;
    _calculatingPointNumber = 1;

    vector<GeometryInfo> geometryHeaderInfo;
    geometryHeaderInfo.resize(_calculatingPointNumber);

    //geometry header
    for(int i = 0; i < _calculatingPointNumber; i++) {
        GeometryInfo geometryHeader;
        geometryHeader.calcPointID = i + 1;
        geometryHeader.calcPoint = origin;
        geometryHeader.calcDirectionNumber = nx * ny * nz;
        geometryHeaderInfo[i] = geometryHeader;
    }
    _geometryHeaderInfo = geometryHeaderInfo;

    //data header
    GeometryInfo dammyGI;
    float scaleFactor;
    dammyGI.calcPoint.resize(3);
    dammyGI.calcPointID = _geometryHeaderInfo[0].calcPointID;
    dammyGI.calcPoint = _geometryHeaderInfo[0].calcPoint;
    scaleFactor = 1.0;
    dammyGI.calcDirectionNumber = _geometryHeaderInfo[0].calcDirectionNumber;

    GeometryInfo geoInfo = this->geometryInfo(0);

    if(geoInfo.calcPointID != dammyGI.calcPointID || geoInfo.calcPoint != dammyGI.calcPoint || geoInfo.calcDirectionNumber != dammyGI.calcDirectionNumber) {
        return false;
    }

    _dataInfo.calcPointID = geoInfo.calcPointID;
    _dataInfo.calcPoint = geoInfo.calcPoint;
    _dataInfo.scaleFactor = scaleFactor;
    _dataInfo.calcDirectionNumber = geoInfo.calcDirectionNumber;
    if(_dataMode == 1) {
        _dataInfo.calcDirectionRec.resize(_dataInfo.calcDirectionNumber);
        for(int i = 0; i < _dataInfo.calcDirectionNumber; i++) {
            _dataInfo.calcDirectionRec[i].directionID = i + 1;
            _dataInfo.calcDirectionRec[i].directionX = phitsData[i].xdata;
            _dataInfo.calcDirectionRec[i].directionY = phitsData[i].ydata;
            _dataInfo.calcDirectionRec[i].directionZ = phitsData[i].zdata;
            _dataInfo.calcDirectionRec[i].deltaX = delX;
            _dataInfo.calcDirectionRec[i].deltaY = delY;
            _dataInfo.calcDirectionRec[i].deltaZ = delZ;
            _dataInfo.calcDirectionRec[i].dirData.resize(_energySpectrumChannelNumber);
            for(int j = 0; j < _energySpectrumChannelNumber; j++) {
                _dataInfo.calcDirectionRec[i].dirData[j] = phitsData[i].ddata[j];
            }
        }
    }
    else {
        return false;
    }

    return true;
}


bool GammaData::write(const string& filename)
{
    ofstream out;
    out.open(filename.data(), ios_base::out | ios_base::binary);
    if(!out) {
        cout << "Binary file was not found." << endl;
        return false;
    }
    filename_ = filename;

    //file header
    out.seekp(0, ios_base::beg);
    out.write((const char *)&_dataMode, 4);
    out.write((const char *)&comment_, 1024);
    out.write((const char *)&unitName_, 8);
    out.write((const char*)&timeUnit_, 4);
    out.write((const char*)&origin_[0], 4);
    out.write((const char*)&origin_[1], 4);
    out.write((const char*)&origin_[2], 4);

    out.write((const char *)&_energySpectrumChannelNumber, 4);
    out.write((const char *)&_energySpectrumMax, 4);
    out.write((const char *)&_energySpectrumMin, 4);
    out.write((const char *)&_scaleFactor, 4);
    out.write((const char *)&_calculatingPointNumber, 4);
    char buf12[976] = "";
    out.write((const char *)&buf12, sizeof(buf12));

    //geometry header
    //out.seekp(2048,ios_base::beg);
    for(int i = 0;i< _calculatingPointNumber;i++) {
        out.write((const char *) &_geometryHeaderInfo[i].calcPointID,4);
        out.write((const char *) &origin_[0],4);
        out.write((const char *) &origin_[1],4);
        out.write((const char *) &origin_[2],4);
        _geometryHeaderInfo[i].calcPoint= origin_;
        out.write((const char *) &_geometryHeaderInfo[i].calcDirectionNumber,4);
        //out.seekp(12,ios_base::cur);
        char buf17[12] = "";
        out.write((const char *)&buf17, sizeof(buf17));
    }
    
    out.close();
    return true;
}


bool GammaData::getDataHeaderInfo(GeometryInfo geoInfo)
{
    ifstream in;
    in.open(filename_.data(),ios_base::in |ios_base::binary);
    if(!in) {
        cout << "Binary file was not found." << endl;
        return false;
    }

    //file header:2048byte
    //geometry header:32*_geometryHeaderInfo.size() byte
    //data header Move : (geoInfo.calcPointID-1) roop ,(32+(28+4*_energySpectrumChannelNumber)*_geometryHeaderInfo[i].calcDirectionNumber) byte
    int seekNum=2048;
    seekNum+=32*_geometryHeaderInfo.size();
    for(int i = 0;i<geoInfo.calcPointID-1;i++) {
        seekNum+=(32+(28+4*_energySpectrumChannelNumber)*_geometryHeaderInfo[i].calcDirectionNumber);
    }

    //data header
    in.seekg(seekNum,ios_base::beg);
    GeometryInfo dammy;
    dammy.calcPoint.resize(3);
    float scaleFactor;
    in.read((char *) &dammy.calcPointID,4);
    in.read((char *) &dammy.calcPoint[0],4);
    in.read((char *) &dammy.calcPoint[1],4);
    in.read((char *) &dammy.calcPoint[2],4);
    in.read((char *) &scaleFactor,4);
    in.read((char *) &dammy.calcDirectionNumber,4);

    if(geoInfo.calcPointID!=dammy.calcPointID || geoInfo.calcPoint!=dammy.calcPoint || geoInfo.calcDirectionNumber != dammy.calcDirectionNumber) {
        in.close();
        return false;
    }

    in.seekg(8,ios_base::cur);
    _dataInfo.calcPointID=geoInfo.calcPointID;
    _dataInfo.calcPoint=geoInfo.calcPoint;
    _dataInfo.scaleFactor=scaleFactor;
    _dataInfo.calcDirectionNumber=geoInfo.calcDirectionNumber;
    if(_dataMode==0) {
        _dataInfo.calcDirectionPo.resize(_dataInfo.calcDirectionNumber);
        for(int i = 0; i < _dataInfo.calcDirectionNumber; i++) {
            in.read((char *) &_dataInfo.calcDirectionPo[i].directionID,4);
            in.read((char *) &_dataInfo.calcDirectionPo[i].phi,4);
            in.read((char *) &_dataInfo.calcDirectionPo[i].lambda,4);
            in.read((char *) &_dataInfo.calcDirectionPo[i].distance,4);
            in.read((char *) &_dataInfo.calcDirectionPo[i].deltaPhi,4);
            in.read((char *) &_dataInfo.calcDirectionPo[i].deltaLambda,4);
            in.read((char *) &_dataInfo.calcDirectionPo[i].deltaDistance,4);
            _dataInfo.calcDirectionPo[i].dirData.resize(_energySpectrumChannelNumber);
            for(int j = 0; j < _energySpectrumChannelNumber; j++) {
                in.read((char *) &_dataInfo.calcDirectionPo[i].dirData[j],4);
            }
        }
    } else if(_dataMode==1) {
        _dataInfo.calcDirectionRec.resize(_dataInfo.calcDirectionNumber);
        for(int i = 0; i < _dataInfo.calcDirectionNumber; i++) {
            in.read((char *) &_dataInfo.calcDirectionRec[i].directionID,4);
            in.read((char *) &_dataInfo.calcDirectionRec[i].directionX,4);
            in.read((char *) &_dataInfo.calcDirectionRec[i].directionY,4);
            in.read((char *) &_dataInfo.calcDirectionRec[i].directionZ,4);
            in.read((char *) &_dataInfo.calcDirectionRec[i].deltaX,4);
            in.read((char *) &_dataInfo.calcDirectionRec[i].deltaY,4);
            in.read((char *) &_dataInfo.calcDirectionRec[i].deltaZ,4);
            _dataInfo.calcDirectionRec[i].dirData.resize(_energySpectrumChannelNumber);
            for(int j = 0; j < _energySpectrumChannelNumber; j++) {
                in.read((char *) &_dataInfo.calcDirectionRec[i].dirData[j],4);
            }
        }
    } else {
        return false;
    }

    in.close();

    return true;
}


bool GammaData::setDataHeaderInfo(GeometryInfo geoInfo)
{
    ofstream out;
    out.open(filename_.data(), ios_base::out | ios_base::binary | ios_base::app);
    if(!out) {
        return false;
    }

    //file header:2048byte
    //geometry header:32*_geometryHeaderInfo.size() byte
    //data header Move : (geoInfo.calcPointID-1) roop ,(32+(28+4*_energySpectrumChannelNumber)*_geometryHeaderInfo[i].calcDirectionNumber) byte
    int seekNum = 2048;
    seekNum += 32 * _geometryHeaderInfo.size();
    for(int i = 0; i < geoInfo.calcPointID - 1; i++) {
        seekNum += (32 + (28 + 4 * _energySpectrumChannelNumber)*_geometryHeaderInfo[i].calcDirectionNumber);
    }

    //data header
    out.seekp(seekNum, ios_base::beg);
    out.write((const char *)&_dataInfo.calcPointID, 4);
    out.write((const char *)&_dataInfo.calcPoint[0], 4);
    out.write((const char *)&_dataInfo.calcPoint[1], 4);
    out.write((const char *)&_dataInfo.calcPoint[2], 4);
    out.write((const char *)&_dataInfo.scaleFactor, 4);
    out.write((const char *)&_dataInfo.calcDirectionNumber, 4);

    if(geoInfo.calcPointID != _dataInfo.calcPointID || geoInfo.calcPoint != _dataInfo.calcPoint || geoInfo.calcDirectionNumber != _dataInfo.calcDirectionNumber) {
        out.close();
        return false;
    }

    //out.seekp(8, ios_base::cur);
    char buf24[8] = "";
    out.write((const char *)&buf24, sizeof(buf24));

    if(_dataMode == 0) {
        for(int i = 0; i < _dataInfo.calcDirectionNumber; i++) {
            out.write((const char *)&_dataInfo.calcDirectionPo[i].directionID, 4);
            out.write((const char *)&_dataInfo.calcDirectionPo[i].phi, 4);
            out.write((const char *)&_dataInfo.calcDirectionPo[i].lambda, 4);
            out.write((const char *)&_dataInfo.calcDirectionPo[i].distance, 4);
            out.write((const char *)&_dataInfo.calcDirectionPo[i].deltaPhi, 4);
            out.write((const char *)&_dataInfo.calcDirectionPo[i].deltaLambda, 4);
            out.write((const char *)&_dataInfo.calcDirectionPo[i].deltaDistance, 4);
            for(int j = 0; j < _energySpectrumChannelNumber; j++) {
                out.write((const char *)&_dataInfo.calcDirectionPo[i].dirData[j], 4);
            }
        }
    } else if(_dataMode == 1) {
        for(int i = 0; i < _dataInfo.calcDirectionNumber; i++) {
            out.write((const char *)&_dataInfo.calcDirectionRec[i].directionID, 4);
            out.write((const char *)&_dataInfo.calcDirectionRec[i].directionX, 4);
            out.write((const char *)&_dataInfo.calcDirectionRec[i].directionY, 4);
            out.write((const char *)&_dataInfo.calcDirectionRec[i].directionZ, 4);
            out.write((const char *)&_dataInfo.calcDirectionRec[i].deltaX, 4);
            out.write((const char *)&_dataInfo.calcDirectionRec[i].deltaY, 4);
            out.write((const char *)&_dataInfo.calcDirectionRec[i].deltaZ, 4);
            for(int j = 0; j < _energySpectrumChannelNumber; j++) {
                out.write((const char *)&_dataInfo.calcDirectionRec[i].dirData[j], 4);
            }
        }
    } else {
        return false;
    }

    out.close();

    return true;
}


void GammaData::addDataInfo(const DataInfo& dataInfo)
{
    for(int i = 0; i < this->dataInfo().calcDirectionNumber; i++) {
        for(int j = 0; j < this->_energySpectrumChannelNumber; j++) {
            this->_dataInfo.calcDirectionRec[i].dirData[j] +=
                dataInfo.calcDirectionRec[i].dirData[j];
        }
    }
}
