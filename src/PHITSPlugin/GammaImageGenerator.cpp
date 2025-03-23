/**
   @author Kenta Suzuki
*/

#include "GammaImageGenerator.h"
#include <cnoid/Camera>
#include <cnoid/EigenUtil>
#include <cnoid/Link>
#include <cnoid/VisualFilter>
#include <QImage>
#include <QPainter>
#include <limits>
#include "Array3D.h"
#include "ComptonCamera.h"
#include "ColorScale.h"
#include "EnergyFilter.h"
#include "GammaCamera.h"
#include "GammaData.h"
#include "PinholeCamera.h"
#include "gettext.h"

using namespace std;
using namespace cnoid;

namespace {

struct GammaDataInfo {

    bool resizeImage(const size_t width, const size_t height) {
        return array.resize(static_cast<unsigned int>(width), static_cast<unsigned int>(height));
    }

    size_t resolutionX() const { return array.size_x(); }
    size_t resolutionY() const { return array.size_y(); }
    double value(const size_t x, const size_t y) const { return array(x, y); }
    void setValue(const size_t x, const size_t y, const double& val) { array(x, y) = val; }
    void clearImage() { array.resize(0); }

    std::tuple<Vector2d, Vector2d> windowsPosition() const {
        return std::make_tuple(top_left_position, right_bottom_position);
    }

    void setWindowsPosition(const Vector2d& top_left, const Vector2d& right_bottom) {
        top_left_position = top_left;
        right_bottom_position = right_bottom;
    }

    std::string name;
    array3d array;
    Vector3d view_position;
    Vector3d view_direction;
    Vector3d up_vector;
    Vector2d top_left_position;
    Vector2d right_bottom_position;
};

template<typename T> T clamp(const T& val, const T& min, const T& max) noexcept
{
    if(val < min) {
        return min;
    }
    if(val > max) {
        return max;
    }
    return val;
}

template<typename T, typename U> T clampCast(const U& val) noexcept
{
    static_assert(is_integral<T>(), "val type must be interger.");

    if(val < static_cast<U>(numeric_limits<T>::min())) {
        return static_cast<U>(numeric_limits<T>::min());
    }
    if(val > static_cast<U>(numeric_limits<T>::max())) {
        return static_cast<U>(numeric_limits<T>::max());
    }
    return val;
}

void drawGammaData(QPainter& painter, const QRect& rect, const GammaDataInfo& dataInfo, ColorScale& scale,
                   const uint8_t transparency, const QPointF& topLeft, const QPointF& bottomRight)
{
    QRect dr(QPoint(topLeft.x()*rect.width(), topLeft.y()*rect.height()), QPoint(bottomRight.x()*rect.width(), bottomRight.y()*rect.height()));

    size_t resX = dataInfo.resolutionX();
    size_t resY = dataInfo.resolutionY();

    uint8_t a = clampCast<uint8_t>(numeric_limits<uint8_t>::max() - transparency);

    QPoint org = dr.topLeft();
    float cellWidth = static_cast<float>(dr.width()) / resX;
    float cellHeight = static_cast<float>(dr.height()) / resY;

    vector<pair<int,int>> xIntrvAry;
    vector<pair<int,int>> yIntrvAry;
    for(size_t i =0 ; i < resX ; ++i) {
        pair<int, int> intrv = make_pair(org.x() + i * cellWidth, org.x() + (i + 1) * cellWidth);
        if(i == 0) {
            xIntrvAry.push_back(intrv);
        } else {
            if(xIntrvAry.back().second + 1 != intrv.first) {
                intrv.first = xIntrvAry.back().second + 1;
            }
            xIntrvAry.push_back(intrv);
        }
    }
    for(size_t j = 0 ; j < resY ; ++j) {
        pair<int, int> intrv = make_pair(org.y()+j*cellHeight, org.y()+(j+1)*cellHeight);
        if(j == 0) {
            yIntrvAry.push_back(intrv);
        } else {
            if(yIntrvAry.back().second+1 != intrv.first) {
                intrv.first = yIntrvAry.back().second + 1;
            }
            yIntrvAry.push_back(intrv);
        }
    }

    for(size_t j = 0 ; j < resY ; ++j) {
        for(size_t i = 0 ; i < resX ; ++i) {
            if(xIntrvAry[i].first > xIntrvAry[i].second || yIntrvAry[j].first > yIntrvAry[j].second) {
                continue;
            }

            double val = dataInfo.value(i,j);
            Vector3 color = scale.linerColor(val);
            QColor qcolor = QColor(color[0] * 255.0, color[1] * 255.0, color[2] * 255.0, 0.5 * 255.0);
            QRect cellRect(QPoint(xIntrvAry[i].first, yIntrvAry[j].first), QPoint(xIntrvAry[i].second, yIntrvAry[j].second));
            painter.fillRect(cellRect, qcolor);
        }
    }
    return;
}

struct DirClippingInfo {
    double tl_x;
    double tl_y;
    double bl_x;
    double bl_y;
    double br_x;
    double br_y;
    double tr_x;
    double tr_y;
    double value;
};

double fovy(double aspectRatio, double fieldOfView)
{
    if(aspectRatio >= 1.0) {
        return fieldOfView;
    } else {
        return 2.0 * atan(tan(fieldOfView / 2.0) / aspectRatio);
    }
}

void getPerspectiveProjectionMatrix(double fovy, double aspect, double zNear, double zFar, Matrix4d& out_matrix)
{
    const double f = 1.0 / tan(fovy / 2.0);
    out_matrix <<
        (f / aspect), 0.0, 0.0, 0.0,
        0.0, f, 0.0, 0.0,
        0.0, 0.0, ((zFar + zNear) /(zNear -zFar)), ((2.0 * zFar * zNear) / (zNear - zFar)),
        0.0, 0.0, -1.0, 0.0;
}

bool isInnerPointTriangle(const double& areaX1, const double& areaY1,
                          const double& areaX2, const double& areaY2,
                          const double& areaX3, const double& areaY3,
                          const double& pointX, const double& pointY)
{

    // 3点の外積計算（z成分）
    double c1 = (areaX1 - areaX3) * (pointY - areaY1) - (areaY1 - areaY3) * (pointX - areaX1);
    double c2 = (areaX2 - areaX1) * (pointY - areaY2) - (areaY2 - areaY1) * (pointX - areaX2);
    double c3 = (areaX3 - areaX2) * (pointY - areaY3) - (areaY3 - areaY2) * (pointX - areaX3);

    if((c1 > 0 && c2 > 0 && c3 > 0) || (c1 < 0 && c2 < 0 && c3 < 0)) {
        // 内側
        return true;
    }
    // 外側
    return false;
}

bool isInnerPointRectangle(const double& areaX1, const double& areaY1,
                           const double& areaX2, const double& areaY2,
                           const double& areaX3, const double& areaY3,
                           const double& areaX4, const double& areaY4,
                           const double& pointX, const double& pointY)
{
    return isInnerPointTriangle(areaX1, areaY1, areaX2, areaY2, areaX3, areaY3, pointX, pointY)
         ||isInnerPointTriangle(areaX1, areaY1, areaX3, areaY3, areaX4, areaY4, pointX, pointY);
}

void setViewingVectors(const Vector3d& camEye, const Vector3d& camUpVec,
        Vector3d& forward, Vector3d& side, Vector3d& up)
{
    forward = camEye.normalized();
    side = forward.cross(camUpVec).normalized();
    up = side.cross(forward);
}

Vector3d polarToRectangular(const double& phi, const double& lambda, const double& r)
{
   Vector3d dist;
   dist.x() = r * cos(phi * TO_RADIAN) * cos(lambda * TO_RADIAN);
   dist.y() = r * cos(phi * TO_RADIAN) * sin(lambda * TO_RADIAN);
   dist.z() = r * sin(phi * TO_RADIAN);
   return dist;
}

Vector3d globalToOpenGL(const Vector3d src)
{
   Vector3d dist;
   dist.x() = -src.y();
   dist.y() = src.z();
   dist.z() = -src.x();
   return dist;
}

Vector3d oepnGLToViewing(const Vector3d& src, const Vector3d& forward,
                         const Vector3d& side, const Vector3d& up)
{
    Vector3d dist;
    dist.x() = src.dot(side);
    dist.y() = src.dot(up);
    dist.z() = -src.dot(forward);
    return dist;
}

Vector4d homogeneousVector(const Vector3d src)
{
    Vector4d dist;
    dist << src.x(), src.y(), src.z(), 1.0;
    return dist;
}

Vector3d viewingToClip(const Vector4d src, const Matrix4d M)
{
    Vector3d dist1;
    Vector4d dist2; //同次座標系

    dist2 = M * src;
    dist1.x() = dist2.x() / dist2[3];
    dist1.y() = dist2.y() / dist2[3];
    dist1.z() = dist2.z() / dist2[3];

    return dist1;
}

void windowsPosition(const uint32_t width, const uint32_t height,
                     const double fov, const double gammaFov,
                     const double camNearClip, const double camFatClip,
                     double& top, double& bottom)
{
    double p = 0.0; double l = 0.0;
    if(width >= height) {
        p = gammaFov / 2.0 * TO_DEGREE;
    } else {
        l = gammaFov / 2.0 * TO_DEGREE;
    }
    Vector3 g = polarToRectangular(p, l, 1.0);
    Vector3 o = globalToOpenGL(g);
    Vector3 f, s, u;
    Vector3 ev;
    ev << 0, 0, -1;
    Vector3 uv;
    uv << 0, 1, 0;
    setViewingVectors(ev, uv, f, s, u);
    Vector3 v = oepnGLToViewing(o, f, s, u);
    double aspectRatio = width / height;
    Matrix4 mat;
    getPerspectiveProjectionMatrix(fovy(aspectRatio, fov), aspectRatio, camNearClip, camFatClip, mat);
    Vector3 c = viewingToClip(homogeneousVector(v), mat);
    double ratio = (width >= height) ? 1.0 - c.y() : 1.0 - c.x();
    top = ratio/2.0;
    bottom = 1 - ratio/2.0;
}

vector<DirClippingInfo> getDirectionsClipping(
        const GammaData& gammaData, const vector<double>& energyFilter,
        const Vector3d& camEyeVec, const Vector3d& camUpVec,
        const double& fov, const double aspectRatio,
        const double& nearClip, const double& farClip)
{
    //視野変換に使用
    Vector3 f, s, u;
    Vector3 ev = globalToOpenGL(camEyeVec);
    Vector3 uv = globalToOpenGL(camUpVec);
    setViewingVectors(ev, uv, f, s, u);
    //透視投影変換に使用
    Matrix4 mat;
    getPerspectiveProjectionMatrix(fovy(aspectRatio, fov), aspectRatio, nearClip, farClip, mat);

    vector<DirClippingInfo> dist;
    int dirCount = gammaData.dataInfo().calcDirectionNumber;
    for(int i = 0; i < dirCount; i++) {

        GammaData::CalcDirectionPoInfo dir = gammaData.dataInfo().calcDirectionPo[i];
//        for(int ig = 0; ig < dir.dirData.size(); ig++) {
//            if(dir.dirData[ig]>0)cout<<ig<<","<<dir.dirData[ig]<<endl;
//        }

        double p1 = dir.phi + dir.deltaPhi / 2.0; double l1 = dir.lambda + dir.deltaLambda / 2.0;
        double p2 = dir.phi - dir.deltaPhi / 2.0; double l2 = dir.lambda + dir.deltaLambda / 2.0;
        double p3 = dir.phi - dir.deltaPhi / 2.0; double l3 = dir.lambda - dir.deltaLambda / 2.0;
        double p4 = dir.phi + dir.deltaPhi / 2.0; double l4 = dir.lambda - dir.deltaLambda / 2.0;

        //極座標 -> 直交座標
        Vector3 gVtl = polarToRectangular(p1, l1, dir.distance); //左上
        Vector3 gVbl = polarToRectangular(p2, l2, dir.distance); //左下
        Vector3 gVbr = polarToRectangular(p3, l3, dir.distance); //右下
        Vector3 gVtr = polarToRectangular(p4, l4, dir.distance); //右上
        //直交座標 -> OpenGL座標
        Vector3 oVtl = globalToOpenGL(gVtl);
        Vector3 oVbl = globalToOpenGL(gVbl);
        Vector3 oVbr = globalToOpenGL(gVbr);
        Vector3 oVtr = globalToOpenGL(gVtr);
        //OpenGL座標 -> 視野座標
        Vector3 vVtl = oepnGLToViewing(oVtl, f, s, u);
        Vector3 vVbl = oepnGLToViewing(oVbl, f, s, u);
        Vector3 vVbr = oepnGLToViewing(oVbr, f, s, u);
        Vector3 vVtr = oepnGLToViewing(oVtr, f, s, u);

        if(vVtl.z() < 0.0 && vVbl.z() < 0.0 && vVbr.z() < 0.0 && vVtr.z() < 0.0) { //4頂点がカメラ正面方向に存在するデータのみ対象とする

            //視野座標 -> クリッピング座標
            Vector3 cVtl = viewingToClip(homogeneousVector(vVtl), mat);
            Vector3 cVbl = viewingToClip(homogeneousVector(vVbl), mat);
            Vector3 cVbr = viewingToClip(homogeneousVector(vVbr), mat);
            Vector3 cVtr = viewingToClip(homogeneousVector(vVtr), mat);

            if((-1.0 < cVtl.x() && cVtl.x() < 1.0 && -1.0 < cVtl.y() && cVtl.y() < 1.0)
              ||(-1.0 < cVbl.x() && cVbl.x() < 1.0 && -1.0 < cVbl.y() && cVbl.y() < 1.0)
              ||(-1.0 < cVbr.x() && cVbr.x() < 1.0 && -1.0 < cVbr.y() && cVbr.y() < 1.0)
              ||(-1.0 < cVtr.x() && cVtr.x() < 1.0 && -1.0 < cVtr.y() && cVtr.y() < 1.0)) { //4頂点のうちいずれかがガンマカメラの画角内に含まれているデータのみ対象とする

                DirClippingInfo dc;
                dc.tl_x = cVtl.x(); dc.tl_y = cVtl.y();
                dc.bl_x = cVbl.x(); dc.bl_y = cVbl.y();
                dc.br_x = cVbr.x(); dc.br_y = cVbr.y();
                dc.tr_x = cVtr.x(); dc.tr_y = cVtr.y();
                dc.value = 0.0;
                //debug
                //cout<<dir.dirData.size()<<endl;
                for(int ig = 0; ig < dir.dirData.size(); ig++) {
                    dc.value += dir.dirData[ig] * energyFilter[ig];
                    //debug
                    //if(dir.dirData[ig]>0)cout<<dir.dirData[ig]<<endl;
                }
                dist.push_back(dc);
            }
        }
    }
    return dist;
}

}

namespace cnoid {

class GammaImageGenerator::Impl
{
public:

    Impl();

    QImage g_qimage;
    Image g_image;

    double effectiveDist;
    Camera* camera;

    void generateImage(Camera* camera, std::shared_ptr<Image>& image);
    void onGenerateGammaImage(Image& image);
    bool setGammaDataInfo(GammaData& gammaData, Vector3d position1);
    bool calc(GammaCamera* camera, const uint32_t widht,
              const uint32_t height, GammaDataInfo& dataInfo, EnergyFilter& filter);
    bool calcPinhole(PinholeCamera* camera, const uint32_t widht,
                     const uint32_t height, GammaDataInfo& dataInfo);
    bool calcCompton(ComptonCamera* camera, const uint32_t widht,
                     const uint32_t height, GammaDataInfo& dataInfo);
};

}


GammaImageGenerator::GammaImageGenerator()
{
    impl = new Impl;
}


GammaImageGenerator::Impl::Impl()
{
    effectiveDist = 1.0;
}


GammaImageGenerator::~GammaImageGenerator()
{
    delete impl;
}


void GammaImageGenerator::generateImage(Camera* camera, std::shared_ptr<Image>& image)
{
    impl->generateImage(camera, image);
}


void GammaImageGenerator::Impl::generateImage(Camera* camera, std::shared_ptr<Image>& image)
{
    if(!image || !camera) {
        return;
    }
    this->camera = camera;

    QImage qImage = toQImage(image.get());
    if(camera) {
        onGenerateGammaImage(*image.get());
        if(!qImage.isNull() && !g_qimage.isNull()) {
            QPainter painter(&qImage);
            painter.setRenderHint(QPainter::Antialiasing, true);
            painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
            painter.drawImage(0, 0, g_qimage);
            painter.end();
        }
    }
    toCnoidImage(image.get(), qImage);
}


void GammaImageGenerator::Impl::onGenerateGammaImage(Image& image)
{
    const auto lclUpVec = Vector3d(0.0, 1.0, 0.0);
    const auto lclEye = Vector3d(0.0, 0.0, -1.0);
    GammaDataInfo dataInfo;
    dataInfo.name = camera->name();
    dataInfo.view_position = camera->link()->T() * camera->localTranslation();
    dataInfo.view_direction = camera->link()->T().linear() * camera->T_local().rotation() * lclEye;
    dataInfo.up_vector = camera->link()->T().linear() * camera->T_local().rotation() * lclUpVec;

    ComptonCamera* comptonCamera = dynamic_cast<ComptonCamera*>(camera);
    PinholeCamera* pinholeCamera = dynamic_cast<PinholeCamera*>(camera);
    if(pinholeCamera) {
       if(pinholeCamera->isReady()) {
           calcPinhole(pinholeCamera, image.width(), image.height(), dataInfo);
        }
    }
    if(comptonCamera) {
        if(comptonCamera->isReady()) {
            calcCompton(comptonCamera, image.width(), image.height(), dataInfo);
        }
    }

    QImage qimage(image.width(),image.height(),QImage::Format_ARGB32);
    for(int j = 0 ; j < image.height() ; ++j) {
        for(int i = 0 ; i < image.width() ; ++i) {
            qimage.setPixel(i, j, qRgba(0, 0, 0, 0));
        }
    }

    if(!qimage.isNull()) {
        QPainter painter(&qimage);
        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
        auto winPos = dataInfo.windowsPosition();
        auto topLeft = get<0>(winPos);
        auto bottomRight = get<1>(winPos);

        double min = 0;
        double max = 0;
        double tmp = 0;
        if(dataInfo.resolutionX() > 0 && dataInfo.resolutionY() > 0) {
            max = dataInfo.value(0, 0);
            min = dataInfo.value(0, 0);
        }
        for(int i = 0; i < dataInfo.resolutionX(); ++i) {
            for(int j = 0; j < dataInfo.resolutionY(); ++j) {
                tmp = dataInfo.value(i, j);
                if(tmp < min) {
                    min = tmp;
                    continue;
                }
                if(max < tmp) {
                    max = tmp;
                    continue;
                }
            }
        }

        ColorScale* scale = new ColorScale();
        // int exp = (int)floor(log10(fabs(max))) + 1;
        // min = 1.0 * pow(10, exp - 6);
        // max = 1.0 * pow(10, exp);
        scale->setRange(min, max);
        static const uint8_t transparency = 0;
        drawGammaData(painter, QRect(0, 0, image.width(), image.height()), dataInfo, *scale,
                      transparency, QPointF(topLeft.x(), topLeft.y()), QPointF(bottomRight.x(), bottomRight.y()));
        painter.end();
    }
    g_qimage = qimage;
}


bool GammaImageGenerator::Impl::setGammaDataInfo(GammaData& gammaData, Vector3d position1)
{
    int pointNumber = gammaData.getCalculatingPointNumber();
    int number = -1;
    bool result = false;
    double mindis = 0;
    Vector3d position2;
    for(int i = 0; i < pointNumber; i++) {
        double dis;
        position2[0] = gammaData.geometryInfo(i).calcPoint[0];
        position2[1] = gammaData.geometryInfo(i).calcPoint[1];
        position2[2] = gammaData.geometryInfo(i).calcPoint[2];
        dis = (position1 - position2).norm();
        if(dis < effectiveDist) {
            if(number == -1 || dis < mindis) {
                number = i;
                mindis = dis;
                result = true;
            }
        }
    }
    if(result) {
        gammaData.getDataHeaderInfo(gammaData.geometryInfo(number));
    }
    return result;
}


bool GammaImageGenerator::Impl::calc(GammaCamera* camera, const uint32_t width,
                           const uint32_t height, GammaDataInfo& dataInfo, EnergyFilter& filter)
{
    GammaData& gammaData = camera->gammaData();
    gammaData.getDataHeaderInfo(gammaData.geometryInfo(0));

    uint32_t resX = camera->resolution()[0];
    uint32_t resY = camera->resolution()[1]; //ガンマカメラの解像度

    double gammaFov = camera->fieldOfView();
    double fov = camera->fieldOfView();
    double nearClip = camera->nearClipDistance();
    double farClip = camera->farClipDistance();

    if(gammaFov > fov) {
        gammaFov = fov;
    } //ガンマカメラの視野角が大きい場合はカメラの視野角と同じとみなす。

    //内部ピクセルの分解能 1ピクセルを 10 * 10 に分割
    int innerPixel = 10;

    dataInfo.resizeImage(resX, resY); //gamDatを初期化

    int channelNumber = gammaData.energySpectrumChannelNumber();

    //エネルギーフィルタ用の配列を作成
    vector<double> energyFilter(channelNumber, 0.0);

    switch (filter.mode()) {
    case EnergyFilter::NO_FILTER:
    {
        for(int i = 0; i < channelNumber; i++) {
            energyFilter[i] = 1.0;
        }
        break;
    }
    case EnergyFilter::RANGE_FILTER:
    {
        int inputMin = filter.min();
        int inputMax = filter.max();

        int wkMin = inputMin;
        int wkMax = inputMax;

        if(inputMin > inputMax) {
            wkMin = inputMax;
            wkMax = inputMin;
        }

        for(int i = 0; i < channelNumber; i++) {
            if(wkMin-1 <= i && i <= wkMax-1) {
                energyFilter[i] = 1.0;
            }
        }
        break;
    }
    case EnergyFilter::NUCLIDE_FILTER:
    {
        for(auto& item : filter.nuclideFilterInfo()) {
            int min = item.min;
            int max = item.max;
            for(int i = 0; i < channelNumber; i++) {
                if(min-1 <= i && i <= max-1) {
                    energyFilter[i] = 1.0;
                }
            }
        }
        break;
    }
    default:
        break;
    }

//    vector<double>  energyFilter = energyFilterVector(channelNumber, engFiltProp);
    //画角に含まれる方向データをクリップ座標系で生成
    vector<DirClippingInfo> dirsClip =
            getDirectionsClipping(gammaData, energyFilter,
                                  dataInfo.view_direction, dataInfo.up_vector, gammaFov, width / height, nearClip, farClip);

    //方向データ（クリップ座標系）から各ピクセル(i,j)の値を計算
    double pointX, pointY, baseX, baseY;
    double pitchX, pitchY, pitchXX, pitchYY;
    pitchX = 2.0 / resX; pitchY = 2.0 / resY;
    pitchXX = 1.0 / resX / innerPixel; pitchYY = 1.0 / resY / innerPixel;
    int nDir = dirsClip.size(); //対象となる方向データ数
    double setValue;
    int innerCnt; //包含カウント

    for(int dirIndex = 0; dirIndex < nDir; dirIndex++) {
        DirClippingInfo dc = dirsClip[dirIndex];
        for(size_t j = 0 ; j < resY ; ++j) {
            baseY = 1.0 - j * pitchY;
            for(size_t i = 0 ; i<resX ; ++i) {
                baseX = -1.0 + i * pitchX;
                innerCnt = 0;
                for(size_t jj = 0; jj < innerPixel; ++jj) {
                    pointY = baseY - (2 * jj + 1) * pitchYY;
                    for(size_t ii = 0; ii < innerPixel; ++ii) {
                        pointX = baseX + (2 * ii + 1) * pitchXX;

                        if(isInnerPointRectangle(dc.tl_x, dc.tl_y, dc.bl_x, dc.bl_y,
                                                  dc.br_x, dc.br_y, dc.tr_x, dc.tr_y,
                                                  pointX, pointY)) {
                            innerCnt++;//内部に含まれている場合カウントアップ

                        }
                    }
                }
                setValue = dc.value * (double) innerCnt/(innerPixel*innerPixel);
                setValue = dataInfo.value(i, j) + setValue;
                dataInfo.setValue(i, j, setValue);
            }
        }
    }

    //ガンマカメラ映像の描画範囲指定
    double top, bottom;
    windowsPosition(width, height, fov, gammaFov, nearClip, farClip, top, bottom);
    dataInfo.setWindowsPosition(Vector2d(top, top), Vector2d(bottom,bottom));

    return true;
}


bool GammaImageGenerator::Impl::calcPinhole(PinholeCamera* camera, const uint32_t width,
                                  const uint32_t height, GammaDataInfo& dataInfo)
{
    GammaData& gammaData = camera->gammaData();
    gammaData.getDataHeaderInfo(gammaData.geometryInfo(0));

    uint32_t resX = camera->resolution()[0];
    uint32_t resY = camera->resolution()[1]; //ガンマカメラの解像度

    double gammaFov = camera->fieldOfView();
    double fov = camera->fieldOfView();
    double nearClip = camera->nearClipDistance();
    double farClip = camera->farClipDistance();

    //内部ピクセルの分解能 1ピクセルを 10 * 10 に分割
    int innerPixel = 10;

    dataInfo.resizeImage(resX, resY); //gamDatを初期化

    int channelNumber = gammaData.energySpectrumChannelNumber();

    GammaData::DataInfo di = gammaData.dataInfo();

    float minx = di.calcDirectionRec[0].directionX;
    float miny = di.calcDirectionRec[0].directionY;
    float minz = di.calcDirectionRec[0].directionZ;

    for(int i = 1; i < di.calcDirectionRec.size(); i++) {
        minx = min(minx, di.calcDirectionRec[i].directionX);
        miny = min(miny, di.calcDirectionRec[i].directionY);
        minz = min(minz, di.calcDirectionRec[i].directionZ);

    }
    int nnz = 0;
    for(int i = 0; i < di.calcDirectionRec.size(); i++) {
        float dx = di.calcDirectionRec[i].deltaX;
        float dy = di.calcDirectionRec[i].deltaY;
        float dz = di.calcDirectionRec[i].deltaZ;
        float x = (di.calcDirectionRec[i].directionX - minx) ;
        float y = (di.calcDirectionRec[i].directionY - miny) ;
        float z = (di.calcDirectionRec[i].directionZ - minz) ;

        int ix = round(x * (1.0 / dx));  // Here be floating point dragons.
        int iz = round(z * (1.0 / dz));
        float d = di.calcDirectionRec[i].dirData[0];
        dataInfo.setValue(ix, iz, d);
    }

    //ガンマカメラ映像の描画範囲指定
    double top, bottom;
    windowsPosition(width, height, fov, gammaFov, nearClip, farClip, top, bottom);
    dataInfo.setWindowsPosition(Vector2d(top, top), Vector2d(bottom, bottom));

    return true;
}


bool GammaImageGenerator::Impl::calcCompton(ComptonCamera* camera, const uint32_t width,
                                  const uint32_t height, GammaDataInfo& dataInfo)
{
    GammaData& gammaData = camera->gammaData();
    gammaData.getDataHeaderInfo(gammaData.geometryInfo(0));

    uint32_t resX = 100;
    uint32_t resY = 100; //ガンマカメラの解像度 ← ここだけがPinholeと異なる

    double gammaFov = camera->fieldOfView();
    double fov = camera->fieldOfView();
    double nearClip = camera->nearClipDistance();
    double farClip = camera->farClipDistance();

    //内部ピクセルの分解能 1ピクセルを 10 * 10 に分割
    int innerPixel = 10;

    dataInfo.resizeImage(resX, resY); //gamDatを初期化

    int channelNumber = gammaData.energySpectrumChannelNumber();

    GammaData::DataInfo di = gammaData.dataInfo();

    float minx = di.calcDirectionRec[0].directionX;
    float miny = di.calcDirectionRec[0].directionY;
    float minz = di.calcDirectionRec[0].directionZ;

    for(int i = 1; i < di.calcDirectionRec.size(); i++) {
        minx = min(minx, di.calcDirectionRec[i].directionX);
        miny = min(miny, di.calcDirectionRec[i].directionY);
        minz = min(minz, di.calcDirectionRec[i].directionZ);

    }
    int nnz = 0;
    for(int i = 0; i < di.calcDirectionRec.size(); i++) {
        float dx = di.calcDirectionRec[i].deltaX;
        float dy = di.calcDirectionRec[i].deltaY;
        float dz = di.calcDirectionRec[i].deltaZ;
        float x = (di.calcDirectionRec[i].directionX - minx);
        float y = (di.calcDirectionRec[i].directionY - miny);
        float z = (di.calcDirectionRec[i].directionZ - minz);

        int ix = round(x * (1.0 / dx));  // Here be floating point dragons.
        int iz = round(z * (1.0 / dz));
        float d = di.calcDirectionRec[i].dirData[0];
        dataInfo.setValue(ix, iz, d);
    }

    //ガンマカメラ映像の描画範囲指定
    double top, bottom;
    windowsPosition(width, height, fov, gammaFov, nearClip, farClip, top, bottom);
    dataInfo.setWindowsPosition(Vector2d(top, top), Vector2d(bottom, bottom));

    return true;
}
