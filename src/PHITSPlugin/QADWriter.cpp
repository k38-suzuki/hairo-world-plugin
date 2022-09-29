/**
   \file
   \author Kenta Suzuki
*/

//#pragma execution_character_set("utf-8")

#include "QADWriter.h"
#include <cnoid/EigenUtil>
#include <cnoid/SceneGraph>
#include <iomanip>
#include <iostream>
#include <sstream>
#include "PHITSRunner.h"

using namespace cnoid;
using namespace std;

namespace {

// 関数オブジェクトを定義する。本来は、ヘッダに入れる。
struct ToUpper {
    char operator()(char c) { return toupper(c); }
};

}


QADWriter::QADWriter()
    : PHITSWriter()
{

}


QADWriter::~QADWriter()
{

}


string QADWriter::writeQAD(GammaData::CalcInfo& calcInfo, int iSrc)
{
    stringstream sstr;
    sstr.str("");

    if(iSrc == 0) {
        if(!searchLink(true)) { return sstr.str(); }
        calcInfo.nSrc = nSource;

        // 原子番号の重複削除
        sort(elementTable.atomicNum().begin(), elementTable.atomicNum().end());
        elementTable.atomicNum().erase(unique(elementTable.atomicNum().begin(), elementTable.atomicNum().end()), elementTable.atomicNum().end());
    }
    const int NCOMP = elementTable.matData().size();
    const int NMAT = elementTable.atomicNum().size();

    // 線源形状の設定
    int NSOPT = -1;
    if(strSrcShape[iSrc] == "SRC_BOX") {
        NSOPT = 1;
        // Boxの場合は、線源の姿勢行列がそのまま回転行列となる
        calcInfo.srcRotMat[iSrc] = srcRotMat[iSrc];
    } else if(strSrcShape[iSrc] == "SRC_CYLINDER") {
        NSOPT = 0;
        // Cylinderの場合は、線源をX(Roll)軸まわりに90°回転させる
        Vector3 rpy = rpyFromRot(srcRotMat[iSrc]); // radian
        for(int i = 0; i < rpy.size(); ++i) {
            rpy[i] = degree(rpy[i]);
        }
        rpy[0] = radian(90.0 + rpy[0]);
        rpy[1] = radian(0.0  + rpy[1]);
        rpy[2] = radian(0.0  + rpy[2]);
        srcRotMat[iSrc] = rotFromRpy(rpy);
        calcInfo.srcRotMat[iSrc] = srcRotMat[iSrc];
    } else if(strSrcShape[iSrc] == "SRC_SPHERE") {
        NSOPT = 2;
        // Sphereの場合は、無回転
        Vector3 rpy;
        rpy[0] = radian(0.0);
        rpy[1] = radian(0.0);
        rpy[2] = radian(0.0);
        srcRotMat[iSrc] = rotFromRpy(rpy);
        calcInfo.srcRotMat[iSrc] = srcRotMat[iSrc];
    } else {
        cout << strSrcShape[iSrc] << " is not defined." << endl;
        sstr.str("");
        return sstr.str();
    }
    Matrix3 invSrcRotMat = srcRotMat[iSrc].inverse();

    // 規格化係数の設定
    double totfact = srcVolume[iSrc];

    // QADの入力ファイル作成
    sstr << fixed;
    sstr << setprecision(4) << "ne= 1" << " emin= " << calcInfo.xyze[3].min << " emax= " << calcInfo.xyze[3].max
        << " TR: x= " << srcCX[iSrc] << " y= " << srcCY[iSrc] << " z= " << srcCZ[iSrc] << endl;
    sstr << LSO[iSrc] << " " << MSO[iSrc] << " " << NSO[iSrc] << " " << NMAT << " " << NCOMP << " 1 " << nEne[iSrc] << " 1 " << NSOPT << " 1 1 0 1 1 1 0" << endl;
    sstr << scientific << setprecision(3) << totfact << " 0.0  0.0  0.0  0.0  0.0  0.0" << endl;

    int divisionX = LSO[iSrc];
    int divisionZ = MSO[iSrc];
    int divisionY = NSO[iSrc];

    // 線源分割
    int nColumn;
    // Cylinder
    if(NSOPT == 0) {
        double deltaR = srcW[iSrc] / 2 / divisionX;
        double deltaZ = srcD[iSrc] / divisionZ;
        double deltaPHI = (360 / divisionY) * M_PI / 180.0;
        double rdiv = 0;
        double zdiv = -srcD[iSrc] / 2;
        double pdiv = 0;

        // x
        nColumn = 0;
        for(int i = 0; i < divisionX + 1; i++) {
            nColumn++;
            sstr << scientific << right << setw(12) << setprecision(4) << i * deltaR + rdiv;
            if(nColumn == 6 || i == divisionX) {
                sstr << endl;
                nColumn = 0;
            }
        }

        // z
        nColumn = 0;
        for(int i = 0; i < divisionZ + 1; i++) {
            nColumn++;
            sstr << scientific << right << setw(12) << setprecision(4) << i * deltaZ + zdiv;
            if(nColumn == 6 || i == divisionZ) {
                sstr << endl;
                nColumn = 0;
            }
        }

        // phi
        nColumn = 0;
        for(int i = 0; i < divisionY + 1; i++) {
            nColumn++;
            sstr << scientific << right << setw(12) << setprecision(4) << i * deltaPHI + pdiv;
            if(nColumn == 6 || i == divisionY) {
                sstr << endl;
                nColumn = 0;
            }
        }
    } else if(NSOPT == 1) { // Box
        double deltaX = srcW[iSrc] / divisionX;
        double deltaZ = srcH[iSrc] / divisionZ;
        double deltaY = srcD[iSrc] / divisionY;
        double xdiv = -srcW[iSrc] / 2;
        double zdiv = -srcH[iSrc] / 2;
        double ydiv = -srcD[iSrc] / 2;

        // x
        nColumn = 0;
        for(int i = 0; i < divisionX + 1; i++) {
            nColumn++;
            sstr << scientific << right << setw(12) << setprecision(4) << i * deltaX + xdiv;
            if(nColumn == 6 || i == divisionX) {
                sstr << endl;
                nColumn = 0;
            }
        }

        // z
        nColumn = 0;
        for(int i = 0; i < divisionZ + 1; i++) {
            nColumn++;
            sstr << scientific << right << setw(12) << setprecision(4) << i * deltaZ + zdiv;
            if(nColumn == 6 || i == divisionZ) {
                sstr << endl;
                nColumn = 0;
            }
        }

        // y
        nColumn = 0;
        for(int i = 0; i < divisionY + 1; i++) {
            nColumn++;
            sstr << scientific << right << setw(12) << setprecision(4) << i * deltaY + ydiv;
            if(nColumn == 6 || i == divisionY) {
                sstr << endl;
                nColumn = 0;
            }
        }
    } else if(NSOPT == 2) { // Sphere
        double deltaR = srcW[iSrc] / 2 / divisionX;
        double deltaTHE = (180 / divisionZ) * M_PI / 180.0;
        double deltaPHI = (360 / divisionY) * M_PI / 180.0;
        double rdiv = 0;
        double tdiv = 0;
        double pdiv = 0;

        // r
        nColumn = 0;
        for(int i = 0; i < divisionX + 1; i++) {
            nColumn++;
            sstr << scientific << setprecision(4) << "  " << i * deltaR + rdiv;
            if(nColumn == 6 || i == divisionX) {
                sstr << endl;
                nColumn = 0;
            }
        }

        // theta
        nColumn = 0;
        sstr << scientific << setprecision(4) << "  " << tdiv;
        for(int i = 0; i < divisionZ + 1; i++) {
            nColumn++;
            sstr << scientific << setprecision(4) << "  " << i * deltaTHE + tdiv;
            if(nColumn == 6 || i == divisionZ) {
                sstr << endl;
                nColumn = 0;
            }
        }

        // phi
        nColumn = 0;
        sstr << scientific << setprecision(4) << "  " << pdiv;
        for(int i = 0; i < divisionY + 1; i++) {
            nColumn++;
            sstr << scientific << setprecision(4) << "  " << i * deltaPHI + pdiv;
            if(nColumn == 6 || i == divisionY) {
                sstr << endl;
                nColumn = 0;
            }
        }
    }

    // Geometry Data
    sstr << "                    GEOMETR DATA" << endl;
    // Cylinder
    if(NSOPT == 0) {
        sstr << "  RCC " << setw(4) << setfill(' ') << 1
            << scientific << setprecision(2)
            << setw(10) << setfill(' ') << 0.0
            << setw(10) << setfill(' ') << 0.0
            << setw(10) << setfill(' ') << -srcD[iSrc] / 2
            << setw(10) << setfill(' ') << 0.0
            << setw(10) << setfill(' ') << 0.0
            << setw(10) << setfill(' ') << srcD[iSrc] << endl;
        sstr << "          " << scientific << setprecision(2)
            << setw(10) << setfill(' ') << srcW[iSrc] / 2 << endl;
    } else if(NSOPT == 1) { // Box
        sstr << "  RPP " << setw(4) << setfill(' ') << 1
            << scientific << setprecision(2)
            << setw(10) << setfill(' ') << -srcW[iSrc] / 2
            << setw(10) << setfill(' ') << srcW[iSrc] / 2
            << setw(10) << setfill(' ') << -srcD[iSrc] / 2
            << setw(10) << setfill(' ') << srcD[iSrc] / 2
            << setw(10) << setfill(' ') << -srcH[iSrc] / 2
            << setw(10) << setfill(' ') << srcH[iSrc] / 2 << endl;
    } else if(NSOPT == 2) { // Sphere
        sstr << "  SPH " << setw(4) << setfill(' ') << 1
            << scientific << setprecision(2)
            << setw(10) << setfill(' ') << 0.0
            << setw(10) << setfill(' ') << 0.0
            << setw(10) << setfill(' ') << 0.0
            << setw(10) << setfill(' ') << srcW[iSrc] / 2 << endl;
    }
    sstr << "  SPH    2         0         0         0    10000." << endl;

    for(int io = 0; io < nObstacle; ++io) {
        // 計算座標系（線源ローカル座標）へ平行移動
        double trX = obsCX[io] - srcCX[iSrc];
        double trY = obsCY[io] - srcCY[iSrc];
        double trZ = obsCZ[io] - srcCZ[iSrc];

        if(strObsShape[io] == "OBS_BOX") {
            // 遮蔽体の中心位置を計算座標系（線源ローカル座標）へ回転（線源の姿勢行列の逆行列を掛ける）
            Vector3 v = { trX, trY, trZ };
            v = invSrcRotMat * v;

            // オブジェクトのローカル座標系
            Vector3 a0 = { -obsW[io] / 2, -obsD[io] / 2, -obsH[io] / 2 }; // (xmin, ymin, zmin)
            Vector3 a1 = {  obsW[io] / 2, -obsD[io] / 2, -obsH[io] / 2 }; // (xmax, ymin, zmin)
            Vector3 a2 = { -obsW[io] / 2,  obsD[io] / 2, -obsH[io] / 2 }; // (xmin, ymax, zmin)
            Vector3 a3 = { -obsW[io] / 2, -obsD[io] / 2,  obsH[io] / 2 }; // (xmin, ymin, zmax)

            // オブジェクトの姿勢行列に線源の姿勢行列の逆行列を掛ける
            Matrix3 calcRotMat = invSrcRotMat * obsRotMat[io];

            // オブジェクトのローカル座標系をQAD計算座標系に変換
            a0 = calcRotMat * a0;
            a1 = calcRotMat * a1 - a0;
            a2 = calcRotMat * a2 - a0;
            a3 = calcRotMat * a3 - a0;
            v += a0;

            for(int i = 0; i < 3; ++i) {
                if(abs(v[i]) < 1e-10) v[i] = 0.0;
                if(abs(a1[i]) < 1e-10) a1[i] = 0.0;
                if(abs(a2[i]) < 1e-10) a2[i] = 0.0;
                if(abs(a3[i]) < 1e-10) a3[i] = 0.0;
            }

            sstr << "  BOX " << setw(4) << setfill(' ') << io + 3
                << scientific << setprecision(2)
                << setw(10) << setfill(' ') << v[0]
                << setw(10) << setfill(' ') << v[1]
                << setw(10) << setfill(' ') << v[2]
                << setw(10) << setfill(' ') << a1[0]
                << setw(10) << setfill(' ') << a1[1]
                << setw(10) << setfill(' ') << a1[2] << endl;
            sstr << "          "
                << setw(10) << setfill(' ') << a2[0]
                << setw(10) << setfill(' ') << a2[1]
                << setw(10) << setfill(' ') << a2[2]
                << setw(10) << setfill(' ') << a3[0]
                << setw(10) << setfill(' ') << a3[1]
                << setw(10) << setfill(' ') << a3[2] << endl;
        } else if(strObsShape[io] == "OBS_CYLINDER") {
            // 遮蔽体の中心位置を計算座標系（線源ローカル座標）へ回転（線源の姿勢行列の逆行列を掛ける）
            Vector3 obsCenter = { trX, trY, trZ };
            obsCenter = invSrcRotMat * obsCenter;

            // オブジェクトのローカル座標系
            Vector3 v = { 0, obsD[io] / 2, 0 }; // 底面中心座標(Vx, Vy, Vz)
            Vector3 h = { 0, -obsD[io], 0 }; // 上面へのベクトル(Hx, Hy, Hz)
            v += obsCenter;
            
            // オブジェクトの姿勢行列に線源の姿勢行列の逆行列を掛ける
            Matrix3 calcRotMat = invSrcRotMat * obsRotMat[io];
            v = calcRotMat * v;
            h = calcRotMat * h;
            for(int i = 0; i < 3; ++i) {
                if(abs(v[i]) < 1e-10) v[i] = 0.0;
                if(abs(h[i]) < 1e-10) h[i] = 0.0;
            }

            sstr << "  RCC " << setw(4) << setfill(' ') << io + 3
                << scientific << setprecision(2)
                << setw(10) << setfill(' ') << v[0]
                << setw(10) << setfill(' ') << v[1]
                << setw(10) << setfill(' ') << v[2]
                << setw(10) << setfill(' ') << h[0]
                << setw(10) << setfill(' ') << h[1]
                << setw(10) << setfill(' ') << h[2] << endl;
            sstr << "          " << setw(10) << setfill(' ') << obsW[io] / 2.0 << endl;
        } else if(strObsShape[io] == "OBS_SPHERE") {
            sstr << "  SPH " << setw(4) << setfill(' ') << io + 3
                << scientific << setprecision(2)
                << setw(10) << setfill(' ') << trX
                << setw(10) << setfill(' ') << trY
                << setw(10) << setfill(' ') << trZ
                << setw(10) << setfill(' ') << obsW[io] / 2.0 << endl;
        }
    }
    sstr << "  END " << endl;

    // Zone set
    sstr << "  ZON     " << setw(7) << setfill(' ') << 1 << endl;
    for(int io = 0; io < nObstacle; ++io) {
        sstr << "  ZON     " << setw(7) << setfill(' ') << io + 3 << endl;;
    }

    sstr << "  ZON     " << setw(7) << setfill(' ') << 2 << setw(7) << setfill(' ') << -1;
    for(int io = 0; io < nObstacle; ++io) {
        sstr << setw(7) << setfill(' ') << -(io + 3);
    }
    sstr << endl;
    sstr << "  END" << endl;

    // mat set
    sstr << "    1    2";
    for(int io = 0; io < nObstacle; ++io) {
        sstr << setw(5) << setfill(' ') << (io + 3);
    }
    sstr << endl;
    sstr << setw(5) << setfill(' ') << srcMaterialId[iSrc];
    for(int io = 0; io < nObstacle; ++io) {
        sstr << setw(5) << setfill(' ') << obsMaterialId[io];
    }
    sstr << setw(5) << setfill(' ') << 1 << endl;

    // Mat Data
    sstr << setw(5) << "   11";

    nColumn = 0;
    for(auto aNum : elementTable.atomicNum()) {
        sstr << setw(5) << setfill(' ') << aNum;
        nColumn++;
        if(nColumn == 10 && elementTable.atomicNum().size() % 10 != 0) {
            sstr << endl;
            sstr << "     ";
            nColumn = 0;
        }
    }
    if(nColumn != 0) sstr << endl;

    // Buildup factor
    transform(buildupName[iSrc].begin(), buildupName[iSrc].end(), buildupName[iSrc].begin(), ToUpper());
    sstr << buildupName[iSrc] << " EXP  0.00  " << buildupName[iSrc] << endl;

    // Material Identification
    int im = 0;
    for(auto& item : elementTable.matData()) {

        int n = get<1>(item);
        double d = get<2>(item);
        int id = 0;
        nColumn = 0;
        for(auto aNum : elementTable.atomicNum()) {
            if(elementTable.elementId()[im][id] == aNum) {
                sstr << scientific << setprecision(4)
                    << setw(12) << setfill(' ') << d * elementTable.weightRate()[im][id];
                if(id != n) id++;
                if(id == n) id = n - 1;
            } else {
                sstr << scientific << setprecision(4)
                    << setw(12) << setfill(' ') << 0.0;
            }

            nColumn++;
            if(nColumn == 6 && elementTable.atomicNum().size() % 6 != 0) {
                sstr << endl;
                nColumn = 0;
            }
        }
        if(nColumn != 0) sstr << endl;
        im++;
    }

    // Source energy
    nColumn = 0;
    double sumActivities = 0.0;
    for(int iEne = 0; iEne < nEne[iSrc]; ++iEne) {
        sstr << scientific << setprecision(4)
            << setw(12) << setfill(' ') << dEnergy[iSrc][iEne];
        nColumn++;
        if(nColumn % 6 == 0) {
            sstr << endl;
            nColumn = 0;
        }
    }
    if(nColumn != 0) sstr << endl;

    // Source Photon Incidence Rate
    nColumn = 0;
    for(int iEne = 0; iEne < nEne[iSrc]; ++iEne) {
        sstr << scientific << setprecision(4)
            << setw(12) << setfill(' ') << dRate[iSrc][iEne] * dActivity[iSrc][iEne];
        nColumn++;
        if(nColumn % 6 == 0) {
            sstr << endl;
            nColumn = 0;
        }
    }
    if(nColumn != 0) sstr << endl;

    // 換算係数
    nColumn = 0;
    for(int i = 0; i < nEne[iSrc]; i++) {
        sstr << scientific << setprecision(4)
            << setw(12) << setfill(' ') << 0;
        nColumn++;
        if(nColumn % 6 == 0) {
            sstr << endl;
            nColumn = 0;
        }
    }
    if(nColumn != 0) sstr << endl;

    //
    nColumn = 0;
    for(int i = 0; i < nEne[iSrc]; i++) {
        sstr << scientific << setprecision(4)
            << setw(12) << setfill(' ') << 0;
        nColumn++;
        if(nColumn % 6 == 0) {
            sstr << endl;
            nColumn = 0;
        }
    }
    if(nColumn != 0) sstr << endl;

    // no eneryg title
    sstr << endl;
    for(int i = 0; i <= int(nEne[iSrc] / 6); i++) {
        sstr << endl;
    }

    sstr << "  PHOTONS    /CM**2/SEC      MR       PER HOUR" << endl;

    int nx = calcInfo.xyze[0].n;
    int ny = calcInfo.xyze[1].n;
    int nz = calcInfo.xyze[2].n;
    double xmin = calcInfo.xyze[0].min * 100;
    double xmax = calcInfo.xyze[0].max * 100;
    double ymin = calcInfo.xyze[1].min * 100;
    double ymax = calcInfo.xyze[1].max * 100;
    double zmin = calcInfo.xyze[2].min * 100;
    double zmax = calcInfo.xyze[2].max * 100;

    for(int iz = 0; iz < nz; iz++) {
        double zlow = (zmin + ((zmax - zmin) / nz * iz));
        double zupp = (zmin + ((zmax - zmin) / nz * (iz + 1)));
        double zgrid = (zupp + zlow) / 2.0 - srcCZ[iSrc];

        for(int j = 0; j < ny; j++) {
            int iy = (ny - 1) - j;
            double ylow = (ymin + ((ymax - ymin) / ny * iy));
            double yupp = (ymin + ((ymax - ymin) / ny * (iy + 1)));
            double ygrid = (yupp + ylow) / 2.0 - srcCY[iSrc];

            for(int ix = 0; ix < nx; ix++) {
                double xlow = (xmin + ((xmax - xmin) / nx * ix));
                double xupp = (xmin + ((xmax - xmin) / nx * (ix + 1)));
                double xgrid = (xupp + xlow) / 2.0 - srcCX[iSrc];

                Vector3 xyz = { xgrid, ygrid, zgrid };
                xyz = invSrcRotMat * xyz;
                for(int i = 0; i < xyz.size(); ++i)
                {
                    if(abs(xyz[i]) < 1e-10) xyz[i] = 0.0;
                }
                if(xyz[0] == 0.0 && xyz[1] == 0.0 && xyz[2] == 0.0) {
                    // QADでは、原点(0,0,0)の計算ができないので、x座標をずらす
                    xyz[0] += 0.01;
                }

                sstr << scientific << setprecision(4)
                    << setw(12) << setfill(' ') << xyz[0]
                    << setw(12) << setfill(' ') << xyz[2]
                    << setw(12) << setfill(' ') << xyz[1]
                    << "     1   0    0   0" << endl;
            }
        }
    }
    sstr << scientific << setprecision(4)
        << setw(12) << setfill(' ') << 0.0
        << setw(12) << setfill(' ') << 0.0
        << setw(12) << setfill(' ') << 0.0
        << "    -1   0    0   0" << endl;

    return sstr.str();
}
