/**
   @author Kenta Suzuki
*/

//#pragma execution_character_set("utf-8")
#include "PHITSWriter.h"
#include <cnoid/BodyItem>
#include <cnoid/EigenUtil>
#include <cnoid/Format>
#include <cnoid/ItemList>
#include <cnoid/MessageView>
#include <cnoid/RootItem>
#include <cnoid/SceneGraph>
#include <cnoid/ValueTree>
#include <iomanip>
#include <iostream>
#include <sstream>
#include "PHITSRunner.h"
#include "gettext.h"

using namespace std;
using namespace cnoid;


PHITSWriter::PHITSWriter()
{
    defaultNuclideTableFile_.clear();
    defaultElementTableFile_.clear();
}


PHITSWriter::~PHITSWriter()
{

}


void PHITSWriter::setCamera(Camera* camera)
{
    ccamera = dynamic_cast<ComptonCamera*>(camera);
    pcamera = dynamic_cast<PinholeCamera*>(camera);
}


void PHITSWriter::setDefaultNuclideTableFile(const string& filename)
{
    defaultNuclideTableFile_ = filename;
    nuclideTable.load(defaultNuclideTableFile_);
}


void PHITSWriter::setDefaultElementTableFile(const string& filename)
{
    defaultElementTableFile_ = filename;
    elementTable.load(defaultElementTableFile_);
}


void PHITSWriter::initialize()
{
    // 初期化
    nSource = 0;
    nEne.clear();
    dEnergy.clear();
    dRate.clear();
    dActivity.clear();
    srcW.clear();
    srcD.clear();
    srcH.clear();
    srcVolume.clear();
    srcCX.clear();
    srcCY.clear();
    srcCZ.clear();
    srcRotMat.clear();
    strSrcShape.clear();
    srcMaterialId.clear();
    srcTotalActivity.clear();

    LSO.clear();
    MSO.clear();
    NSO.clear();
    buildupName.clear();

    nObstacle = 0;
    obsW.clear();
    obsD.clear();
    obsH.clear();
    obsCX.clear();
    obsCY.clear();
    obsCZ.clear();
    obsRotMat.clear();
    strObsShape.clear();
    obsMaterialId.clear();

    materialRho.clear();
    resolution << 8, 8;
}


bool PHITSWriter::searchLink(bool flagQAD)
{
    initialize();

    // リンクの検索
    //
    RootItem* rootItem = RootItem::instance();
    ItemList<BodyItem> checked = rootItem->checkedItems<BodyItem>();

    for(const auto& item : checked) {
        // RadiationSourceの取得
        Body* body = item->body();
        for(Link* link : body->links()) {
            const Mapping* info = link->info();

            // for radiation source
            {
                // 核種名指定 or エネルギー指定
                ValueNode* nuclideNameNode = info->find("nuclide");
                ValueNode* energyNode = info->find("energy");
                if(nuclideNameNode->isValid() || energyNode->isValid()) {
                    MessageView::instance()->putln(formatR(_("{0} has been detected."), link->name()));
                    if(nuclideNameNode->isValid()) {
                        int nNuc;
                        vector<string> strNucNames;
                        vector<double> nucActictities;
                        // 核種名と濃度の取得
                        int offSet = 2;
                        if(nuclideNameNode != nullptr && nuclideNameNode->isListing()) {
                            Listing* list = nuclideNameNode->toListing();
                            nNuc = list->size() / offSet;
                            std::vector<std::string> tmpNucNames;
                            std::vector<double> tmpActivities;
                            for(int i = 0; i < nNuc; ++i) {
                                strNucNames.push_back(list->get(i * offSet).toString());
                                nucActictities.push_back((double)list->get(i * offSet + 1).toDouble());
                            }
                        }
                        // エネルギー指定に合わせる
                        nEne.push_back(0);
                        std::vector<double> tmpEnergy;
                        std::vector<double> tmpRate;
                        std::vector<double> tmpActivity;
                        for(int iNuc = 0; iNuc < nNuc; ++iNuc) {
                            int id = 0;
                            bool flgNuc = false;
                            for(auto& item : nuclideTable.nuclideName()) {
                                string NucName = get<0>(item);
                                int nEnergy = get<1>(item);

                                if(strNucNames[iNuc] == NucName) {
                                    nEne[nSource] += nEnergy;
                                    flgNuc = true;
                                    for(int iEne = 0; iEne < nEnergy; iEne++) {
                                        tmpEnergy.push_back(nuclideTable.sourceEnergy()[id][iEne]);
                                        tmpRate.push_back(nuclideTable.sourceIncidenceRate()[id][iEne]);
                                        tmpActivity.push_back(nucActictities[iNuc]);
                                    }
                                    break;
                                }
                                id++;
                            }
                            if(flgNuc == false) {
                                cout << strNucNames[iNuc] << " is not found." << endl;
                                return false;
                            }
                        }
                        dEnergy.push_back(tmpEnergy);
                        dRate.push_back(tmpRate);
                        dActivity.push_back(tmpActivity);
                    } else if(energyNode->isValid()) {
                        // エネルギー、放出割合、濃度の取得
                        int offSet = 3;
                        if(energyNode != nullptr && energyNode->isListing()) {
                            Listing* list = energyNode->toListing();
                            nEne.push_back(list->size() / offSet);
                            std::vector<double> tmpEnergy;
                            std::vector<double> tmpRate;
                            std::vector<double> tmpActivity;
                            for(int i = 0; i < nEne[nSource]; ++i) {
                                tmpEnergy.push_back((double)list->get(i * offSet).toDouble());
                                tmpRate.push_back((double)list->get(i * offSet + 1).toDouble());
                                tmpActivity.push_back((double)list->get(i * offSet + 2).toDouble());
                            }
                            dEnergy.push_back(tmpEnergy);
                            dRate.push_back(tmpRate);
                            dActivity.push_back(tmpActivity);
                        }
                    }
                    double sumActivities = 0;
                    for(int i = 0; i < nEne[nSource]; ++i) {
                        sumActivities += dActivity[nSource][i] * dRate[nSource][i];
                    }
                    srcTotalActivity.push_back(sumActivities);

                    //
                    // 共通ノードの読み込み
                    //
                    // ***** sourceShape *****
                    ValueNode* sourceShapeNode = info->find({ "object_type", "objectType" });
                    if(!sourceShapeNode->isValid()) {
                        return false;
                    } else {
                        strSrcShape.push_back(sourceShapeNode->toString());
                    }

                    // ***** materialId *****
                    ValueNode* srcMaterialIdNode = info->find("materialId");
                    ValueNode* materialNode = info->find("material");
                    if(srcMaterialIdNode->isValid()) {
                        srcMaterialId.push_back(srcMaterialIdNode->toDouble());
                    } else if(materialNode->isValid()) {
                        string name = materialNode->toString();
                        int materialId = elementTable.materialId(name);
                        srcMaterialId.push_back(materialId);
                    } else {
                        return false;
                    }

                    // リンクの中心座標の取得
                    const Isometry3 position = link->position();
                    Vector3 translation = position.translation();
                    srcCX.push_back(translation.x() * 100); // [m]->[cm]
                    srcCY.push_back(translation.y() * 100);
                    srcCZ.push_back(translation.z() * 100);
                    // リンクの境界の取得
                    SgNode* node1 = link->shape();
                    const BoundingBox &bb1 = node1->boundingBox();
                    // 回転角度の取得
                    srcRotMat.push_back(link->R());
                    //const Vector3 rpy = rpyFromRot(mat1); // radian

                    srcW.push_back((bb1.max().x() - bb1.min().x()) * 100);
                    srcD.push_back((bb1.max().y() - bb1.min().y()) * 100);
                    srcH.push_back((bb1.max().z() - bb1.min().z()) * 100);

                    if(item->findRootItem() == nullptr) {
                        link = nullptr;
                    }

                    // srcVolumeの計算
                    if(strSrcShape[nSource] == "SRC_BOX") {
                        srcVolume.push_back(srcW[nSource] * srcD[nSource] * srcH[nSource]);
                    } else if(strSrcShape[nSource] == "SRC_CYLINDER") {
                        srcVolume.push_back(pow(srcW[nSource] / 2, 2) * srcD[nSource] * M_PI);
                    } else if(strSrcShape[nSource] == "SRC_SPHERE") {
                        srcVolume.push_back((4.0 / 3.0) * M_PI * pow(srcW[nSource] / 2, 3));
                    } else {
                        return false;
                    }

                    // QADパラメータの読み込み
                    if(flagQAD) {
                        // only used in QAD
                        // 線源分割数
                        ValueNode* divisionNode = info->find({ "source_division", "sourceDivision" });
                        if(!divisionNode->isValid()) {
                            cout << "sourceDivision node is not found." << endl;
                            return false;
                        }
                        // 線源分割数の取得
                        if(divisionNode != nullptr && divisionNode->isListing()) {
                            Listing* list = divisionNode->toListing();
                            LSO.push_back(list->get(0).toInt()); // R(cyl.),  X(cart.), ρ(spher.)
                            NSO.push_back(list->get(1).toInt()); // φ(cyl.), Y(cart.), φ(spher.)
                            MSO.push_back(list->get(2).toInt()); // Z(cyl.),  Z(cart.), θ(spher.)
                        }

                        ValueNode* buildupNode = info->find({ "buildup_factor", "buildupFactor" });
                        if(!buildupNode->isValid()) {
                            cout << "buildupFactor node is not found." << endl;
                            return false;
                        }
                        buildupName.push_back(buildupNode->toString());
                    }

                    nSource += 1;
                }
            }

            // for obstacle
            {
                ValueNode* obstacleShapeNode = info->find({ "object_type", "objectType" });
                if(obstacleShapeNode->isValid()) {
                    string obstacle = obstacleShapeNode->toString();
                    if(obstacle == "OBS_BOX" || obstacle == "OBS_CYLINDER" || obstacle == "OBS_SPHERE") {
                        strObsShape.push_back(obstacleShapeNode->toString());
                        // リンクの中心座標の取得
                        const Isometry3 position = link->position();
                        Vector3 translation = position.translation();
                        obsCX.push_back(translation.x() * 100); // [m]->[cm]
                        obsCY.push_back(translation.y() * 100);
                        obsCZ.push_back(translation.z() * 100);
                        // リンクの境界の取得
                        SgNode* node1 = link->shape();
                        const BoundingBox &bb1 = node1->boundingBox();
                        // 回転角度の取得
                        obsRotMat.push_back(link->R());

                        obsW.push_back((bb1.max().x() - bb1.min().x()) * 100);
                        obsD.push_back((bb1.max().y() - bb1.min().y()) * 100);
                        obsH.push_back((bb1.max().z() - bb1.min().z()) * 100);

                        ValueNode* materialIdNode = info->find("materialId");
                        ValueNode* materialNode = info->find("material");

                        // Material IDの取得
                        if(materialIdNode->isValid()) {
                            obsMaterialId.push_back(materialIdNode->toDouble());
                        } else if(materialNode->isValid()) {
                            string name = materialNode->toString();
                            int materialId = elementTable.materialId(name);
                            obsMaterialId.push_back(materialId);
                        } else {
                            return false;
                        }

                        nObstacle += 1;
                    }
                }
            }
        }
    }

    return true;
}


bool PHITSWriter::searchCameraLink(const int inputMode)
{
    Link* clink = nullptr;
    Vector3 localT;
    if(inputMode == GammaData::PINHOLE) {
        // ピンホールカメラの取得
        string material = pcamera->material();
        detMaterialId = elementTable.materialId(material);
        detMatThickness = pcamera->thickness();
        detPinholeOpening = pcamera->pinholeOpening();

        clink = pcamera->link();
        localT = pcamera->localTranslation();

        // ピンホールカメラのパラメータ設定
        resolution = pcamera->resolution();
        detectorSize = 10; // unit: cm
        shieldThickness = detMatThickness;
        angle = degree(pcamera->fieldOfView()); // radian
        pinholeOpening = detPinholeOpening;
        distance = (detectorSize / 2.0) / tan(radian(angle) / 2.0);
    } else if(inputMode == GammaData::COMPTON) {
        // コンプトンカメラの取得
        string material = ccamera->material();
        detMaterialId = elementTable.materialId(material);
        detElementWidth = ccamera->elementWidth();
        detScattererThickness = ccamera->scattererThickness();
        detAbsorberThickness = ccamera->absorberThickness();
        detDistance = ccamera->distance();
        detArm = ccamera->arm();

        clink = ccamera->link();
        localT = ccamera->localTranslation();

        // コンプトンカメラのパラメータ設定
        resolution = ccamera->resolution();
        xminCC = -(detElementWidth + elementDistance * 2) * resolution[0] / 2;
        xmaxCC = (detElementWidth + elementDistance * 2) * resolution[0] / 2;
        zminCC = -(detElementWidth + elementDistance * 2) * resolution[1] / 2;
        zmaxCC = (detElementWidth + elementDistance * 2) * resolution[1] / 2;
        xminEU = xminCC + elementDistance;
        xmaxEU = xminEU + detElementWidth;
        zminEU = zminCC + elementDistance;
        zmaxEU = zminEU + detElementWidth;
        xminRU = xminCC;
        xmaxRU = xmaxEU + elementDistance;
        zminRU = zminCC;
        zmaxRU = zmaxEU + elementDistance;
        yminSC = -detScattererThickness;
        ymaxSC = 0.0;
        ymaxAB = yminSC - detDistance;
        yminAB = ymaxAB - detAbsorberThickness;
        yairSC = ymaxSC + airThicknes;
        yairAB = ymaxAB + airThicknes;
    }

    if(clink == nullptr) {
        return false;
    }

    //
    // カメラ位置・姿勢の取得
    //
    // リンクの中心座標の取得
    const Isometry3 position = clink->position();
    Vector3 translation = position.translation();
    translation += localT;
    detCX = translation.x() * 100; // [m]->[cm]
    detCY = translation.y() * 100;
    detCZ = translation.z() * 100;
    // 回転角度の取得
    detRotMat = clink->R();
    Vector3 rpy = rpyFromRot(detRotMat); // radian
    for(int i = 0; i < rpy.size(); ++i) {
        detRpy[i] = degree(rpy[i]);
    }
    // 回転行列の作成
    rpy[0] = radian(-detRpy[0]);
    rpy[1] = radian(-detRpy[1]);
    rpy[2] = radian(90.0 - detRpy[2]);
    Matrix3 p;
    p = rotFromRpy(rpy);
    //
    // 座標変換
    // 1. Detector
    Vector3 xyz;
    xyz[0] = detCX;
    xyz[1] = detCY;
    xyz[2] = detCZ;
    xyz = p * xyz;
    detCX = xyz[0];
    detCY = xyz[1];
    detCZ = xyz[2];
    // 2. Source
    for(int is = 0; is < nSource; ++is) {
        xyz[0] = srcCX[is];
        xyz[1] = srcCY[is];
        xyz[2] = srcCZ[is];
        xyz = p * xyz;
        srcCX[is] = xyz[0] - detCX;
        srcCY[is] = xyz[1] - detCY;
        srcCZ[is] = xyz[2] - detCZ;
        srcRotMat[is] = p * srcRotMat[is];
    }
    // 3. Obstacle
    for(int io = 0; io < nObstacle; ++io) {
        xyz[0] = obsCX[io];
        xyz[1] = obsCY[io];
        xyz[2] = obsCZ[io];
        xyz = p * xyz;
        obsCX[io] = xyz[0] - detCX;
        obsCY[io] = xyz[1] - detCY;
        obsCZ[io] = xyz[2] - detCZ;
        obsRotMat[io] = p * obsRotMat[io];
    }

    return true;
}


string PHITSWriter::writePHITS(GammaData::CalcInfo calcInfo)
{
    PHITSRunner phits;
    stringstream sstr;
    sstr.str("");

    // RadiationSource及びObstacleリンクの探索
    if(!searchLink()) return sstr.str();
    if(calcInfo.inputMode != GammaData::DOSERATE) {
        // ガンマカメラリンクの探索
        if(!searchCameraLink(calcInfo.inputMode)) {
            return sstr.str();
        }
    }

    // PHITSの入力ファイル作成
    //********************************************************************************
    // ***** Title *****
    sstr << "[ T i t l e ]" << endl;
    if(calcInfo.inputMode == GammaData::DOSERATE) {
        sstr << "PHITS Input File" << endl;
    } else if(calcInfo.inputMode == GammaData::PINHOLE) {
        sstr << "Pinhole Camera" << endl;
    } else if(calcInfo.inputMode == GammaData::COMPTON) {
        sstr << "Compton Camera" << endl;
    }
    sstr << "" << endl;

    //********************************************************************************
    // ***** Parameters *****
    sstr << "[ P a r a m e t e r s ]" << endl;
    sstr << " icntl    =           0     # (D=0) 3:ECH 5:NOR 6:SRC 7,8:GSH 11:DSH 12:DUMP" << endl;
    sstr << " maxcas   =        " << calcInfo.maxcas << "     # (D=10) number of particles per one batch" << endl;
    sstr << " maxbch   =        " << calcInfo.maxbch << "     # (D=10) number of batches" << endl;
    if(calcInfo.inputMode == GammaData::COMPTON) {
        sstr << " e-mode  =       1" << endl;
    }
    sstr << " itall    =           1     # (D=0) 0:no tally at batch, 1:same, 2:different" << endl;
    sstr << " file(1)  = " << phits.installPath() << endl;
    sstr << " file(6)  = phits.out       # (D=phits.out) general output file name" << endl;
    sstr << "" << endl;

    //sstr << "set: c1[" << fixed << setprecision(2) << c1 << "]";
    //sstr << " c2[" << c2 << "]";
    //sstr << " c3[" << c3 << "]" << endl;
    if(calcInfo.inputMode == GammaData::PINHOLE) {
        sstr << "set:c11[ " << std::fixed << std::setprecision(3) << detectorSize << " ] # Detector Size" << std::endl;
        sstr << "set:c12[ " << std::fixed << std::setprecision(3) << shieldThickness << " ] # Thickness" << std::endl;
        sstr << "set:c13[ " << std::fixed << std::setprecision(3) << distance << " ] # Distance from focus to detector" << std::endl;
        sstr << "set:c14[ " << std::fixed << std::setprecision(5) << angle << " ] # Field of View" << std::endl;
        sstr << "set:c15[ " << std::fixed << std::setprecision(3) << pinholeOpening << " ] # Pinhole Opening" << std::endl;
    } else if(calcInfo.inputMode == GammaData::COMPTON) {
        sstr << "set:c11[ " << std::fixed << std::setprecision(3) << xminCC << " ]" << std::endl;
        sstr << "set:c12[ " << std::fixed << std::setprecision(3) << xmaxCC << " ]" << std::endl;
        sstr << "set:c13[ " << std::fixed << std::setprecision(3) << zminCC << " ]" << std::endl;
        sstr << "set:c14[ " << std::fixed << std::setprecision(3) << zmaxCC << " ]" << std::endl;
        sstr << "set:c15[ " << std::fixed << std::setprecision(3) << xminEU << " ]" << std::endl;
        sstr << "set:c16[ " << std::fixed << std::setprecision(3) << xmaxEU << " ]" << std::endl;
        sstr << "set:c17[ " << std::fixed << std::setprecision(3) << zminEU << " ]" << std::endl;
        sstr << "set:c18[ " << std::fixed << std::setprecision(3) << zmaxEU << " ]" << std::endl;
        sstr << "set:c19[ " << std::fixed << std::setprecision(3) << xminRU << " ]" << std::endl;
        sstr << "set:c20[ " << std::fixed << std::setprecision(3) << xmaxRU << " ]" << std::endl;
        sstr << "set:c21[ " << std::fixed << std::setprecision(3) << zminRU << " ]" << std::endl;
        sstr << "set:c22[ " << std::fixed << std::setprecision(3) << zmaxRU << " ]" << std::endl;
        sstr << "set:c23[ " << std::fixed << std::setprecision(3) << yminSC << " ]" << std::endl;
        sstr << "set:c24[ " << std::fixed << std::setprecision(3) << ymaxSC << " ]" << std::endl;
        sstr << "set:c25[ " << std::fixed << std::setprecision(3) << yminAB << " ]" << std::endl;
        sstr << "set:c26[ " << std::fixed << std::setprecision(3) << ymaxAB << " ]" << std::endl;
        sstr << "set:c27[ " << std::fixed << std::setprecision(3) << yairSC << " ]" << std::endl;
        sstr << "set:c28[ " << std::fixed << std::setprecision(3) << yairAB << " ]" << std::endl;
    }
    sstr << "" << endl;

    //********************************************************************************
    // ***** Source *****
    sstr << "[ S o u r c e ]" << endl;

    double totfact = 0.0;
    for(int is = 0; is < nSource; ++is) {
        // subSourceの計算
        double subSource = srcVolume[is] * srcTotalActivity[is];

        totfact += subSource;

        sstr << scientific << setprecision(4);
        sstr << " <Source> =   " << subSource << "           # weight of this sub-source" << endl;
        sstr << "   s-type =   2                # axial source  with energy spectrum" << endl;
        sstr << "     proj =  photon            # kind of incident particle" << endl;
        sstr << fixed << setprecision(4);
        sstr << "       x0 =   -" << srcW[is] << "/2            # minimum position of x-axis [cm]" << endl;
        sstr << "       x1 =    " << srcW[is] << "/2            # maximum position of x-axis [cm]" << endl;
        sstr << "       y0 =   -" << srcD[is] << "/2            # minimum position of y-axis [cm]" << endl;
        sstr << "       y1 =    " << srcD[is] << "/2            # maximum position of y-axis [cm]" << endl;
        sstr << "       z0 =   -" << srcH[is] << "/2            # minimum position of z-axis [cm]" << endl;
        sstr << "       z1 =    " << srcH[is] << "/2            # maximum position of z-axis [cm]" << endl;
        sstr << "     trcl = " << is + 2 << endl;
        sstr << "      dir =   all              # z-direction of beam [cosine]" << endl;
        if(strSrcShape[is] == "SRC_BOX") {
            //sstr << "      reg = 2" << endl; // BOXの場合、角度によってはPHITSでエラーになるため削除
        } else {
            sstr << "      reg = " << is + 2 << endl;
        }
        if(calcInfo.inputMode == GammaData::COMPTON) {
            int jEne = 0;
            double dRate_max = 0.0;
            for(int iEne = 0; iEne < nEne[is]; ++iEne) {
                if(dRate_max <= dRate[is][iEne]) {
                    dRate_max = dRate[is][iEne];
                    jEne = iEne;
                };
            }
            energy_ = dEnergy[is][jEne];
            sstr << "       e0 = " << scientific << setprecision(4) << dEnergy[is][jEne] << "         # number of energy and weight" << endl;
        } else {
            sstr << "   e-type =   8                # pointwise energies given by data" << endl;
            sstr << "       ne =    " << nEne[is] << "               # number of energy and weight" << endl;
            sstr << scientific << setprecision(4);
            for(int iEne = 0; iEne < nEne[is]; ++iEne) {
                sstr << "       " << dEnergy[is][iEne] << "   "
                    << dActivity[is][iEne] << "*" << srcVolume[is] << "*" << dRate[is][iEne] << endl;
            }
        }
        sstr << "" << endl;

    }

    sstr << "  totfact =   " << scientific << setprecision(4) << totfact << endl;
    sstr << "" << endl;

    //********************************************************************************
    // ***** Material *****
    sstr << "[ M a t e r i a l ]" << endl;
    // Material Identification
    int id = 1;
    int im = 0;

    for(auto& item : elementTable.matData()) {
        string s = get<0>(item);
        int n = get<1>(item);
        double d = get<2>(item);

        materialRho.push_back(d);

        sstr << "$ " << s << " D = "
            << scientific << setprecision(4) << d << " g/cm3" << endl;
        sstr << "m" << id << setw(10) << setfill(' ') << elementTable.element()[im][0]
            << scientific << setprecision(4)
            << setw(15) << setfill(' ') << -elementTable.weightRate()[im][0] << endl;
        for(int j = 1; j < n; j++) {
            sstr << "  " << setw(10) << setfill(' ') << elementTable.element()[im][j]
                << scientific << setprecision(4)
                << setw(15) << setfill(' ') << -elementTable.weightRate()[im][j] << endl;
        }
        id++;
        im++;
    }
    sstr << fixed << setprecision(2);
    sstr << "" << endl;

    //********************************************************************************
    // ***** Surface *****
    sstr << "[ S u r f a c e ]" << endl;
    sstr << "   1       so     10000." << endl;
    for(int is = 0; is < nSource; ++is) {
        if(strSrcShape[is] == "SRC_BOX") {
            sstr << "   " << is + 2 << "   " << is + 2 << "   rpp"
                << "   " << -srcW[is] / 2 << "  " << srcW[is] / 2
                << "   " << -srcD[is] / 2 << "  " << srcD[is] / 2
                << "   " << -srcH[is] / 2 << "  " << srcH[is] / 2 << endl;
        } else if(strSrcShape[is] == "SRC_CYLINDER") {
            sstr << "   " << is + 2 << "   " << is + 2 << "   rcc"
                << "   " << 0 << "  " << -srcD[is] / 2 << "  " << 0 // P(x0,y0,z0)
                << "   " << 0 << "  " << srcD[is] << "  " << 0      // H(Hx,Hy,Hz)
                << "   " << srcW[is] / 2 << endl;                   // R
        } else if(strSrcShape[is] == "SRC_SPHERE") {
            sstr << "   " << is + 2 << "   " << is + 2 << "   so"
                << "   " << srcW[is] / 2 << endl;
        }
    }
    if(calcInfo.inputMode == GammaData::PINHOLE) {
        sstr << "c Pinhole Camera" << std::endl;
        sstr << " 101            rpp     -c11/2      c11/2      -c13     -c12/2  -c11/2     c11/2" << std::endl;
        sstr << " 102            rpp     -c11/2-c12  c11/2+c12  -c13-c12  c12/2  -c11/2-c12 c11/2+c12" << std::endl;
        sstr << " 103            ky       0.0 tan(c14/2*pi/180)**2" << std::endl;
        sstr << " 104            py      -c12/2" << std::endl;
        sstr << " 105            py       c12/2" << std::endl;
        sstr << " 106            cy       c15/2" << std::endl;
    } else if(calcInfo.inputMode == GammaData::COMPTON) {
        sstr << "c 散乱体シンチレータアレイ" << std::endl;
        sstr << " 101    rpp  c15  c16  c23  c24  c17  c18" << std::endl;
        sstr << " 102    rpp  c19  c20  c23  c27  c21  c22" << std::endl;
        sstr << " 103    rpp  c11  c12  c23  c27  c13  c14" << std::endl;
        sstr << "c" << std::endl;
        sstr << "c 吸収体シンチレータアレイ" << std::endl;
        sstr << " 111    rpp  c15  c16  c25  c26  c17  c18" << std::endl;
        sstr << " 112    rpp  c19  c20  c25  c28  c21  c22" << std::endl;
        sstr << " 113    rpp  c11  c12  c25  c28  c13  c14" << std::endl;
    }
    for(int io = 0; io < nObstacle; ++io) {
        if(io == 0) sstr << "c Obstacles" << std::endl;
        if(strObsShape[io] == "OBS_BOX") {
            sstr << " " << io + 201 << " " << io + 201 << "   rpp    "
                << -obsW[io] / 2 << " "
                << obsW[io] / 2 << " "
                << -obsD[io] / 2 << " "
                << obsD[io] / 2 << " "
                << -obsH[io] / 2 << " "
                << obsH[io] / 2 << endl;
        } else if(strObsShape[io] == "OBS_CYLINDER") {
            sstr << " " << io + 201 << " " << io + 201 << "   rcc    "
                << 0.0 << " "
                << -obsD[io] / 2 << " "
                << 0.0 << " "
                << 0.0 << " "
                << obsD[io] << " "
                << 0.0 << " "
                << obsW[io] / 2 << endl;
        } else if(strObsShape[io] == "OBS_SPHERE") {
            sstr << " " << io + 201 << " " << io + 201 << "   so    "
                << obsW[io] / 2 << endl;
        }
    }
    sstr << "" << endl;

    //********************************************************************************
    // ***** Transform *****
    sstr << "[ Transform ]" << endl;
    //sstr << " *TR1 " << x << " " << y << " " << z << " " << rpyZ << " " << rpyY << " " << rpyX << "   0.0 0.0 0.0    0.0 0.0 0.0    2" << endl; // M=2の方法ではうまくいかない
    for(int is = 0; is < nSource; ++is) {
        sstr << " TR" << is + 2 << "  " << setprecision(4)
            << srcCX[is] << " " << srcCY[is] << " " << srcCZ[is] << " "
            << srcRotMat[is](0, 0) << " " << srcRotMat[is](1, 0) << " " << srcRotMat[is](2, 0) << " "
            << srcRotMat[is](0, 1) << " " << srcRotMat[is](1, 1) << " " << srcRotMat[is](2, 1) << " "
            << srcRotMat[is](0, 2) << " " << srcRotMat[is](1, 2) << " " << srcRotMat[is](2, 2) << " "
            << " 1" << endl;
    }
    for(int io = 0; io < nObstacle; ++io) {
        sstr << " TR" << io + 201 << "  "
            << obsCX[io] << " " << obsCY[io] << " " << obsCZ[io] << " "
            << obsRotMat[io](0, 0) << " " << obsRotMat[io](1, 0) << " " << obsRotMat[io](2, 0) << " "
            << obsRotMat[io](0, 1) << " " << obsRotMat[io](1, 1) << " " << obsRotMat[io](2, 1) << " "
            << obsRotMat[io](0, 2) << " " << obsRotMat[io](1, 2) << " " << obsRotMat[io](2, 2) << " "
            << " 1" << endl;

    }
    sstr << "" << endl;

    //********************************************************************************
    // ***** Cell *****
    sstr << "[ C e l l ]" << endl;
    sstr << "   1    -1            1             $ outer region" << endl;
    for(int is = 0; is < nSource; ++is) {
        int id = srcMaterialId[is];
        sstr << "   " << is + 2 << "   " << id << " " << -materialRho[id - 1] << " " << -(is + 2) << endl;
    }
    if(calcInfo.inputMode == GammaData::PINHOLE) {
        sstr << "c Pinhole Camera" << std::endl;
        sstr << " 101          1  -1.21e-3        -101                          $ Inner Box (air)" << std::endl;
        sstr << " 102          " << detMaterialId << "  " << -materialRho[detMaterialId - 1]
            << "         101  -102  #103 #104         $ Outer Box" << std::endl;
        sstr << " 103          1  -1.21e-3        -103   104  -105 #104         $ Pinhole Cone" << std::endl;
        sstr << " 104          1  -1.21e-3        -106   104  -105              $ Pinhole Opening" << std::endl;
    } else if(calcInfo.inputMode == GammaData::COMPTON) {
        sstr << "c 散乱体シンチレータアレイ" << std::endl;
        sstr << " 101     " << detMaterialId << "  " << -materialRho[detMaterialId - 1] << "     -101  u=1" << std::endl;
        sstr << " 102     1  -1.21e-3   101  u=1" << std::endl;
        sstr << " 103     0            -102  u=2" << std::endl;
        sstr << "                            lat = 1 fill = 0:" << resolution[0] - 1 << " 0:0 0:" << resolution[1] - 1 << std::endl;
        for(int i = 0; i < resolution[1]; i++) {
            for(int j = 0; j < resolution[0]; j++) {
                if(j == 0) {
                    sstr << "                            1";
                } else if(j == resolution[0] - 1) {
                    sstr << "  1" << std::endl;
                } else {
                    sstr << "  1";
                }
            }
        }
        sstr << " 104     0               -103  fill = 2" << std::endl;
        sstr << "c" << std::endl;
        sstr << "c 吸収体シンチレータアレイ" << std::endl;
        sstr << " 111     " << detMaterialId << "  " << -materialRho[detMaterialId - 1] << "     -111  u=11" << std::endl;
        sstr << " 112     1  -1.21e-3   111  u=11" << std::endl;
        sstr << " 113     0            -112  u=12" << std::endl;
        sstr << "                            lat = 1 fill = 0:" << resolution[0] - 1 << " 0:0 0:" << resolution[1] - 1 << std::endl;
        for(int i = 0; i < resolution[1]; i++) {
            for(int j = 0; j < resolution[0]; j++) {

                if(j == 0) {
                    sstr << "                            11";
                } else if(j == resolution[0] - 1) {
                    sstr << " 11" << std::endl;
                } else {
                    sstr << " 11";
                }
            }
        }
        sstr << " 114     0            -113  fill=12" << std::endl;
    }
    for(int io = 0; io < nObstacle; ++io) {
        if(io == 0) sstr << "c Obstacles" << std::endl;
        int id = obsMaterialId[io];
        sstr << " " << io + 201 << " " << "    " << id << " " << -materialRho[id - 1] << " "
            << -(io + 201) << endl;
    }
    sstr << "c Inner area" << std::endl;
    sstr << " 999     1 -1.21e-3   -1";
    for(int is = 0; is < nSource; ++is) {
        sstr << "  #" << is + 2;
    }
    if(calcInfo.inputMode == GammaData::PINHOLE) {
        sstr << "  #101 #102 #103 #104";
    } else if(calcInfo.inputMode == GammaData::COMPTON) {
        sstr << "  #104 #114";
    }

    for(int io = 0; io < nObstacle; ++io) {
        sstr << "  #" << io + 201;
    }
    sstr << endl;
    sstr << "" << endl;

    //********************************************************************************
    // ***** Tally *****
    if(calcInfo.inputMode == GammaData::DOSERATE) {
        sstr << "[ T - T r a c k ]" << endl;
        sstr << "    title = Dose in xyz mesh" << endl;
        sstr << "     mesh =  xyz            # mesh type is xyz scoring mesh" << endl;
        sstr << "   x-type =    2            # x-mesh is linear given by xmin, xmax and nx" << endl;
        sstr << "       nx =    " << calcInfo.xyze[0].n << endl;
        sstr << "     xmin =    " << calcInfo.xyze[0].min * 100 << endl;
        sstr << "     xmax =    " << calcInfo.xyze[0].max * 100 << endl;
        sstr << "   y-type =    2            # y-mesh is given by the below data" << endl;
        sstr << "       ny =    " << calcInfo.xyze[1].n << endl;
        sstr << "     ymin =    " << calcInfo.xyze[1].min * 100 << endl;
        sstr << "     ymax =    " << calcInfo.xyze[1].max * 100 << endl;
        sstr << "   z-type =    2            # z-mesh is linear given by zmin, zmax and nz" << endl;
        sstr << "       nz =    " << calcInfo.xyze[2].n << endl;
        sstr << "     zmin =    " << calcInfo.xyze[2].min * 100 << endl;
        sstr << "     zmax =    " << calcInfo.xyze[2].max * 100 << endl;
        sstr << "   e-type =    2            # e-mesh is given by the below data" << endl;
        sstr << "       ne =    " << calcInfo.xyze[3].n << endl;
        sstr << "     emin =    " << calcInfo.xyze[3].min << endl;
        sstr << "     emax =    " << calcInfo.xyze[3].max << endl;
        sstr << "     unit =    1            # unit is [1/cm^2/source]" << endl;
        sstr << "     axis =   xy            # axis of output" << endl;
        sstr << "     file = dose_xy.out    # file name of output for the above axis" << endl;
        sstr << "     part =  all" << endl;
        sstr << "    gshow =    2            # 0: no 1:bnd, 2:bnd+mat, 3:bnd+reg 4:bnd+lat" << endl;
        sstr << "   epsout =    1            # (D=0) generate eps file by ANGEL" << endl;
        sstr << "   factor = 1e-6*3600       # [pSv/sec] -> [uSv/h]" << endl;
        sstr << "    z-txt = Dose [uSv/h]" << endl;
        sstr << "" << endl;
        sstr << "  multiplier = all          # number of material group" << endl;
        sstr << "      mat      mset1   " << endl;
        sstr << "      all (1.0 -200) " << endl;
        sstr << "" << endl;
    } else if(calcInfo.inputMode == GammaData::PINHOLE) {
        sstr << "[ T - C r o s s ]" << endl;
        sstr << "     mesh = xyz" << endl;
        sstr << "   x-type =    2            # x-mesh is linear given by xmin, xmax and nx" << endl;
        sstr << "     xmin = -c11/2            # minimum value of x-mesh points" << endl;
        sstr << "     xmax =  c11/2            # maximum value of x-mesh points" << endl;
        sstr << "       nx =  " << resolution[0] << "            # number of x-mesh points" << endl;
        sstr << "   y-type =    2            # y-mesh is linear given by ymin, ymax and ny" << endl;
        sstr << "     ymin =  -c13-0.5           # minimum value of y-mesh points" << endl;
        sstr << "     ymax =  -c13+0.5           # maximum value of y-mesh points" << endl;
        sstr << "       ny =    1            # number of y-mesh points" << endl;
        sstr << "   z-type =    2            # z-mesh is linear given by zmin, zmax and nz" << endl;
        sstr << "     zmin =  -c11/2           # minimum value of z-mesh points" << endl;
        sstr << "     zmax =   c11/2           # maximum value of z-mesh points" << endl;
        sstr << "       nz =   " << resolution[1] << "            # number of z-mesh points" << endl;
        sstr << "   e-type =    2            # e-mesh is log given by emin, emax and ne" << endl;
        sstr << "     emin =   0.0           # minimum value of e-mesh points" << endl;
        sstr << "     emax =   3.0           # maximum value of e-mesh points" << endl;
        sstr << "       ne =   1            # number of e-mesh points" << endl;
        sstr << "     unit = 1" << endl;
        sstr << "     axis = xz" << endl;
        sstr << "     file = cross_xz.out" << endl;
        sstr << "     part = all" << endl;
        sstr << "  2d-type = 3" << endl;
        sstr << "    gshow = 1 " << endl;
        sstr << "   epsout = 1" << endl;
        sstr << "    resol = 1" << endl;
        sstr << "   output = current    " << endl;
        sstr << "   enclos = 1" << endl;
    } else if(calcInfo.inputMode == GammaData::COMPTON) {
        sstr << "[Counter]" << std::endl;
        sstr << " counter = 1" << std::endl;
        sstr << "    part = photon" << std::endl;
        sstr << "    reg  in  out  coll  ref" << std::endl;
        sstr << "    101  1   0    0     0" << std::endl;
        sstr << " counter = 2" << std::endl;
        sstr << "    part = photon" << std::endl;
        sstr << "    reg  in  out  coll  ref" << std::endl;
        sstr << "    101  0   0    1     0" << std::endl;
        sstr << " counter = 3" << std::endl;
        sstr << "    part = photon" << std::endl;
        sstr << "    reg  in  out  coll  ref" << std::endl;
        sstr << "    102  0   0    1     0" << std::endl;
        sstr << "    112  0   0    1     0" << std::endl;
        sstr << "    999  0   0    1     0" << std::endl;
        sstr << "      2  0   0    1     0" << std::endl;
        for(int io = 0; io < nObstacle; ++io) {
            sstr << "    " << io + 201 << "  0   0    1     0" << std::endl;
        }
        sstr << "" << std::endl;

        sstr << "[T - Cross]" << std::endl;
        sstr << "     mesh = reg" << std::endl;
        sstr << "      reg = 1" << std::endl;
        sstr << "       r-from r-to  area" << std::endl;
        sstr << "       112    111   1" << std::endl;
        sstr << "     part = photon" << std::endl;
        sstr << "     unit = 1" << std::endl;
        sstr << "   e-type = 1" << std::endl;
        sstr << "       ne = 40" << std::endl;
        sstr << "             0.00E+00 5.00E-02 1.00E-01 1.50E-01 2.00E-01" << std::endl;
        sstr << "             2.50E-01 3.00E-01 3.50E-01 4.00E-01 4.50E-01" << std::endl;
        sstr << "             5.00E-01 5.50E-01 6.00E-01 6.50E-01 7.00E-01" << std::endl;
        sstr << "             7.50E-01 8.00E-01 8.50E-01 9.00E-01 9.50E-01" << std::endl;
        sstr << "             1.00E+00 1.05E+00 1.10E+00 1.15E+00 1.20E+00" << std::endl;
        sstr << "             1.25E+00 1.30E+00 1.35E+00 1.40E+00 1.45E+00" << std::endl;
        sstr << "             1.50E+00 1.55E+00 1.60E+00 1.65E+00 1.70E+00" << std::endl;
        sstr << "             1.75E+00 1.80E+00 1.85E+00 1.90E+00 1.95E+00" << std::endl;
        sstr << "             2.00E+00" << std::endl;
        sstr << "     axis = eng" << std::endl;
        sstr << "     file = flux_cross.out" << std::endl;
        sstr << "    gshow = 1            # 0: no 1 : bnd, 2 : bnd + mat, 3 : bnd + reg 4 : bnd + lat" << std::endl;
        sstr << "   epsout = 2             # (D = 0) generate eps file by ANGEL" << std::endl;
        sstr << "     dump = -11" << std::endl;
        sstr << "             1 2 3 4 5 6 7 8 11 12 13" << std::endl;
        sstr << " ctmin(1) = 1" << std::endl;
        sstr << " ctmin(2) = 1" << std::endl;
        sstr << " ctmax(2) = 1" << std::endl;
        sstr << " ctmin(3) = 0" << std::endl;
        sstr << " ctmax(3) = 0" << std::endl;
    }
    sstr << "" << endl;

    sstr << "[END]" << endl;
    sstr << "" << endl;

    return sstr.str();
}
