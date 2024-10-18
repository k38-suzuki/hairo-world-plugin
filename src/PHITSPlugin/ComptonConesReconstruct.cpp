/**
   @author Kenta Suzuki
*/

#include "ComptonConesReconstruct.h"
#include <math.h>
#include <vector>
#include "ComptonCone.h"

using namespace std;
using namespace cnoid;

namespace {

const int PROJECTION_TYPE_INDEX_EQUISOLIDANGLE = 2;
const double VALUE_OUT_OF_RANGE = 0.0;

class Rainbow
{
public:

    void setRainbow(double min, double max)
    {
        min_ = min;
        max_ = max;
        diff_ = max - min;
    }

    void get(double value, int id, vector<vector<int>> &iRGB)
    {
        if(diff_ == 0.0) {
            iRGB[id][0] = 0;
            iRGB[id][1] = 0;
            iRGB[id][2] = 0;
        } else {
            double h = (value - min_) / diff_;
            double s = 1.0;
            double v = 1.0;

            h = (1.0 - h) * 240.0 / 360.0;

            //double[] rgb = ColorSpace.hsv2rgb(h, s, v);
            vector<double>rgb(3,0);

            hsv2rgb(rgb, h, s, v);
            //Color.getRGB(rgb);

            //        (int)floor(rgb[0] * 255.0),
            //        (int)floor(rgb[1] * 255.0),
            //        (int)floor(rgb[2] * 255.0)
            //};
            iRGB[id][0] = (int)floor(rgb[0] * 255.0);
            iRGB[id][1] = (int)floor(rgb[1] * 255.0);
            iRGB[id][2] = (int)floor(rgb[2] * 255.0);

        }
    }

    void hsv2rgb(vector<double> &rgb, double h, double s, double v)
    {
        double r = v;
        double g = v;
        double b = v;

        if(s > 0.0) {
            h = h * 6.0;
            int i = (int)floor(h);
            double f = h - i;

            switch (i) {
            case 0:
                g = g * (1.0 - s * (1.0 - f));
                b = b * (1.0 - s);
                break;

            case 1:
                r = r * (1.0 - s * f);
                b = b * (1.0 - s);
                break;

            case 2:
                r = r * (1.0 - s);
                b = b * (1.0 - s * (1.0 - f));
                break;

            case 3:
                r = r * (1.0 - s);
                g = g * (1.0 - s * f);
                break;

            case 4:
                r = r * (1.0 - s * (1.0 - f));
                g = g * (1.0 - s);
                break;

            case 5:
                g = g * (1.0 - s);
                b = b * (1.0 - s * f);
                break;

            default:
                g = g * (1.0 - s * (1.0 - f));
                b = b * (1.0 - s);
                break;
            }
        }

        rgb[0] = r;
        rgb[1] = g;
        rgb[2] = b;
    }

private:
    double min_;
    double max_;
    double diff_;
};

class AngleResponse
{
public:

    void setAngleResponse(int ndiv)
    {
        //this->nx = 0;
        //this->ny = 0;
        this->nx = ndiv;
        this->ny = ndiv;
        this->size = ndiv * ndiv;
    }

    void read()
    {
        for(int i = 0; i < this->size; i++) {
            this->values.push_back(1.0);
        }
    }

    double getValueByIndex(int ix, int iy)
    {
        if(ix < 0 || ix >= this->nx) { return VALUE_OUT_OF_RANGE; }
        if(iy < 0 || iy >= this->ny) { return VALUE_OUT_OF_RANGE; }

        return this->values[iy * this->nx + ix];
    }

    double getValue(double dx, double dy, double cx, double cy)
    {
        if(dx < -cx || dx > cx) { return VALUE_OUT_OF_RANGE; }
        if(dy < -cy || dy > cy) { return VALUE_OUT_OF_RANGE; }

        //double ixf = (dx + 1.0) / 2.0 * this.nx;
        //double iyf = (-dy + 1.0) / 2.0 * this.ny;
        double ixf = (dx + cx) / (cx * 2.0) * (nx - 1.0);
        double iyf = (-dy + cy) / (cy * 2.0) * (ny - 1.0);

        int ix1 = (int)floor(ixf);
        int ix2 = ix1 + 1;
        int iy1 = (int)floor(iyf);
        int iy2 = iy1 + 1;

        double vx1y1 = getValueByIndex(ix1, iy1);
        double vx1y2 = getValueByIndex(ix1, iy2);
        double vx2y1 = getValueByIndex(ix2, iy1);
        double vx2y2 = getValueByIndex(ix2, iy2);

        double vy1 = vx1y1 + (vx2y1 - vx1y1) * (ixf - ix1);
        double vy2 = vx1y2 + (vx2y2 - vx1y2) * (ixf - ix1);

        double v = vy1 + (vy2 - vy1) * (iyf - iy1);

        //    if(Debug) {
        //        Console.WriteLine(
        //            "" + dx + ", " + dy + ", " + ixf + ", " + iyf + ", " +
        //            ix1 + ", " + ix2 + ", " + iy1 + ", " + iy2 + ", " +
        //            vx1y1 + ", " + vx1y2 + ", " + vx2y1 + ", " + vx2y2 + ", " +
        //            vy1 + ", " + vy2 + ", " + v);
        //    }

        //cout << "v " << v << endl;
        return v;
    }

protected:
    int nx;
    int ny;
    int size;
    vector<double> values;
};

class NoAngleResponse : AngleResponse
{
public:
    //    override public double getValueByIndex(int ix, int iy)
    double getValueByIndex(int ix, int iy)
    {
        if(ix < 0 || ix >= this->nx) {
            return VALUE_OUT_OF_RANGE;
        }
        if(iy < 0 || iy >= this->ny) {
            return VALUE_OUT_OF_RANGE;
        }

        if(this->values[iy * this->nx + ix] == 0.0) {
            return 0.0;
        } else {
            return 1.0;
        }
    }
};

}

namespace cnoid {

class EquisolidAngleProjection
{
public:

     void setEquisolidAngleProjection(int nx, int ny);
     void getHalfSphereCoordByIndex(vector<double> &sph, int ix, int iy, double cx, double cy);
     void getProjectedPlaneCoordByIndex(vector<double> &rxy, int ix, int iy, double cx, double cy);
     void getProjectedPlaneCoordByHalfSpherePoint(vector<double> &rxy, vector<double> sph);
     void convertProjPlaneCoordToIndex(double rx, double ry, double cx, double cy);
     //public: int getNearestIndexOnProjPlane(double rx, double ry)
     void getNearestIndexOnProjPlane(vector<int> &spidx, double rx, double ry, double cx, double cy);
     double theta2r(double theta, double cx);
     static double r2theta(double r, double cx);
     static double theta2sph(double theta);
     void getIXIY(vector<double> &ipt);

     const double HalfPI = M_PI * 0.5;  // pi/2
     const double Root2 = sqrt(2.0);    // sqrt(2)

private:
     int _nx;
     int _ny;
     double _ix;
     double _iy;
     int _ix1;
     int _ix2;
     int _iy1;
     int _iy2;
     double _rx;
     double _ry;
};


class EquidistanceProjection
{
public:

    void setEquidistanceProjection(int nx, int ny)
    {
        this->_nx = nx;
        this->_ny = ny;
    }

    bool getHalfSphereCoordByIndex(std::vector<double> &sph, int ix, int iy, double cx, double cy)
    {
        if(ix < 0) { ix = 0; }
        if(ix > this->_nx) { ix = this->_nx; }
        if(iy < 0) { iy = 0; }
        if(iy > this->_ny) { iy = this->_ny; }

        //double rx = 2.0 / _nx * (ix + 0.5) - 1.0;
        //double ry = 2.0 / _ny * (iy + 0.5) - 1.0;
        double rx = (cx*2.0) / this->_nx * (ix + 0.5) - cx;
        double ry = (cy*2.0) / this->_ny * (iy + 0.5) - cy;
        double rxy = sqrt(rx * rx + ry * ry);

        if(rxy == 0.0) {
            sph[0] = 0.0;
            sph[1] = 0.0;
            sph[2] = 1.0;
            return false;
                //return new double[] { 0.0, 0.0, 1.0 };
        } else if(rxy > cx) {
            return true;
        } else {

            double thetaxy = r2theta(rxy, cx);

            double z = cos(thetaxy);
            double sqrtz = sqrt(1 - z * z);

            //return sph;
            sph[0] = rx / rxy * sqrtz;
            sph[1] = ry / rxy * sqrtz;
            sph[2] = z;
            return false;
        }
    }

    double theta2r(double theta, double cx)
    {
        if(theta <= -HalfPI) { return -cx; }
        if(theta >= HalfPI) { return cx; }

        return theta / HalfPI;
    }

    static double r2theta(double r, double cx)
    {
        if(r <= -cx) { return -M_PI / 2; }
        if(r >= cx) { return M_PI / 2; }

        return r * M_PI / 2;
    }

    const double HalfPI = M_PI * 0.5;       // pi/2
    const double Root2 = sqrt(2.0);       // sqrt(2)

private:
    int _nx;
    int _ny;
};

}


void ComptonConesReconstruct::setndiv(int ndiv, double theta)
{
    this->_ndiv = ndiv;
    this->_nx = ndiv - 1;
    this->_ny = ndiv - 1;
    this->_cx = theta;
    this->_cy = theta;
}


void ComptonConesReconstruct::Exec(vector<double> &values, double camera_width, double camera_height, double sphere_radius,
                                   double arm, int cnt, vector<double> x1, vector<double> z1, vector<double> x2,
                                   vector<double> z2, double Ga, vector<double> th, vector<int> iflg)
{
    EquisolidAngleProjection eaproj;
    AngleResponse response;
    ComptonCone coneValue;

    eaproj.setEquisolidAngleProjection(this->_nx, this->_ny);
    response.setAngleResponse(this->_ndiv);

    const int ndiv = this->_ndiv;
    const int nxy = (this->_nx + 1) * (this->_ny + 1);

    double cameraXOffset = camera_width * 0.5;
    double cameraYOffset = camera_height * 0.5;

    //const char *respfilepath = "angle_response.tsv";
    //response->read(respfilepath);
    response.read();

    //double[] rsum = Enumerable.Repeat<double>(0.0, (nx + 1) * (ny + 1)).ToArray();
    for(int ic = 0, nic = cnt; ic < nic; ic++) {
        if(iflg[ic] == 1) {
            double ScatterX =  x1[ic] - cameraXOffset;
            double ScatterY = -z1[ic] + cameraYOffset;
            double AbsorbX  =  x2[ic] - cameraXOffset;
            double AbsorbY  = -z2[ic] + cameraYOffset;
            double Gapsa = Ga;
            double Angle = th[ic] / M_PI * 180.0;

            //var coneValue = cameraCone.getCone(ic);
            coneValue.setPosition(ScatterX, ScatterY);
            coneValue.setDirection(ic, ScatterX, ScatterY, AbsorbX, AbsorbY, Gapsa);
            coneValue.setHAngle(Angle);

            // 等立体角射影の平面投影座標上の点がコンプトンコーンのARM角内に入っているかどうかを調べる。
            //int[] bin = Enumerable.Repeat<int>(0, (nx + 1) * (ny + 1)).ToArray();
            vector<int> bin(nxy,0);

            int id = 0;
            for(int iy = 0; iy <= this->_ny; iy++) {
                for(int ix = 0; ix <= this->_nx; ix++) {
                    //double sph = eaproj.getHalfSphereCoordByIndex(ix, iy);    // 単位ベクトル
                    vector<double> sph(3,0);                                // 単位ベクトル
                    eaproj.getHalfSphereCoordByIndex(sph, ix, iy, this->_cx, this->_cy);

                    sph[0] = sphere_radius * sph[0];       // 半球の半径をsphere_radiusとする。sphは半球上の(x,y,z)座標となる。
                    sph[1] = sphere_radius * sph[1];
                    sph[2] = sphere_radius * sph[2];

                    if(coneValue.isPointContainedInArm(sph, arm) == true) {
                        bin[id] = 1;
                    }
                    id++;
                }
            }

            // ARM角内に入っているとき、その感度補正値の和を取る。
            id = 0;
            for(int iy = 0; iy <= this->_ny; iy++) {
                for(int ix = 0; ix <= this->_nx; ix++) {
                    if(bin[id] == 1) {

                        //double *rxy = eaproj->getProjectedPlaneCoordByIndex(ix, iy);
                        vector<double> rxy(2,0);
                        eaproj.getProjectedPlaneCoordByIndex(rxy, ix, iy, this->_cx, this->_cy);

                        //cout << "rxy0 " << rxy[0] << endl;
                        //cout << "rxy1 " << rxy[1] << endl;

                        //double rvalue = response.getValue(rxy[0], rxy[1]);
                        double rvalue = 0.0;
                        rvalue = response.getValue(rxy[0], rxy[1], this->_cx, this->_cy);

                        //cout << "id " << id << endl;
                        //cout << "rvalue " << rvalue << endl;

                        values[id] += rvalue;
                        //cout << "rsum[id] " << rsum[id] << endl;
                    }

                    id++;
                }
            }
        }
    }

    //this.NumCones = comptonData.getCount();
    //return *rsum;
}


ReconstructedImage::ReconstructedImage()
{
    white << 255.0, 255.0, 255.0;
    black << 0.0, 0.0, 0.0;
    dgray << 64.0, 64.0, 64.0;
    gray << 128.0, 128.0, 128.0;
    lgray << 192.0, 192.0, 192.0;
    llgray << 224.0, 224.0, 224.0;
}


ReconstructedImage::~ReconstructedImage()
{

}


void ReconstructedImage::SetImageSize(int ndiv, int imageSize, double theta)
{
    this->_ndiv = ndiv;
    this->_imageWidth = imageSize;
    this->_imageHeight = imageSize;
    this->_cx = theta;
    this->_cy = theta;
}


void ReconstructedImage::CreateImage(vector<vector<int>> &imageRgb, vector<double> values,
                                     int projectionTypeIndex, double displayRegionCoeffX, double displayRegionCoeffY)
{
    int imageWidth = this->_imageWidth;
    int imageHeight = this->_imageHeight;
    int ndiv = this->_ndiv;

    //static const int row = (this->imageWidth) * (this->imageHeight);
    //int imageRgb[10000][3] = {};

    double rmin = 1.0E20;
    double rmax = 0.0;
    vector<double> rvalues(imageWidth * imageHeight,0);

    // 画像点の値を取得する。
    if(projectionTypeIndex == PROJECTION_TYPE_INDEX_EQUISOLIDANGLE) {
        calculatePixelPointValueOnEquisolidAngleProjection(ndiv, imageWidth, imageHeight, values, displayRegionCoeffX, displayRegionCoeffY, rvalues);
    } else {
        calculatePixelPointValueOnEquidistanceProjection(ndiv, imageWidth, imageHeight, values, rvalues);
    }

    // 最小値、最大値の取得
    int index = 0;
    for(int ipy = 0; ipy < imageHeight; ipy++) {
        for(int ipx = 0; ipx < imageWidth; ipx++) {
            if(rvalues[index] > 0.0) {
                if(rvalues[index] < rmin) { rmin = rvalues[index]; }
                if(rvalues[index] > rmax) { rmax = rvalues[index]; }
            }

            index++;
        }
    }

    //CompCones.Rainbow rainbow = new CompCones.Rainbow(rmin, rmax);     // レインボースケール
    Rainbow rainbow;
    rainbow.setRainbow(rmin, rmax);     // レインボースケール

    // RGBのセット
    // imageRgb[0] は画像左上のピクセルのRGB。
    index = 0;
    for(int ipy = 0; ipy < imageHeight; ipy++) {
        for(int ipx = 0; ipx < imageWidth; ipx++) {
            if(rvalues[index] > 0.0) {
                //imageRgb[ipy * imageWidth + ipx] = rainbow.get(rvalues[index]);
                rainbow.get(rvalues[index], ipy * imageWidth + ipx, imageRgb);

                //cout << "RGB_CALC " << ipx << " " << ipy << endl;
                //cout << "RGB0 " << imageRgb[ipy * imageWidth + ipx][0] << endl;
                //cout << "RGB1 " << imageRgb[ipy * imageWidth + ipx][1] << endl;
                //cout << "RGB2 " << imageRgb[ipy * imageWidth + ipx][2] << endl;
                //system("pause");
            } else {
                //imageRgb[ipy * imageWidth + ipx] = RGB_WHITE;
                imageRgb[ipy * imageWidth + ipx][0] = white[0];
                imageRgb[ipy * imageWidth + ipx][1] = white[1];
                imageRgb[ipy * imageWidth + ipx][2] = white[2];

                //cout << "RGB_WHITE" << ipx << " " << ipy << endl;
                //cout << "RGB0 " << imageRgb[ipy * imageWidth + ipx][0] << endl;
                //cout << "RGB1 " << imageRgb[ipy * imageWidth + ipx][1] << endl;
                //cout << "RGB2 " << imageRgb[ipy * imageWidth + ipx][2] << endl;
                //system("pause");
            }

            index++;
        }
    }
    //return **imageRgb;
}


void ReconstructedImage::calculatePixelPointValueOnEquisolidAngleProjection(
        int ndiv, int imageWidth, int imageHeight, vector<double> values,
        double displayRegionCoeffX, double displayRegionCoeffY, vector<double> &rvalues)
{
    EquisolidAngleProjection eaproj;
    eaproj.setEquisolidAngleProjection(ndiv, ndiv);

    int index = 0;
    for(int ipy = 0; ipy < imageHeight; ipy++) {
        for(int ipx = 0; ipx < imageWidth; ipx++) {
            //double rx = 2.0 / (imageWidth - 1) * ipx - 1.0;
            //double ry = 2.0 / (imageHeight - 1) * ipy - 1.0;
            double rx = (this->_cx * 2.0) / (imageWidth - 1) * ipx - this->_cx;
            double ry = (this->_cy * 2.0) / (imageHeight - 1) * ipy - this->_cy;

            rx = displayRegionCoeffX * rx;
            ry = displayRegionCoeffY * ry;

            //int[] spidx = eaproj.getNearestIndexOnProjPlane(rx, ry);
            vector<int> spidx(4,0);

            eaproj.getNearestIndexOnProjPlane(spidx, rx, ry, this->_cx, this->_cy);

            int id = spidx[2] * ndiv + spidx[0];      // 本来なら線形補間すべき。とりあえず (ix1,iy1)の値を使う。

            rvalues[index] = values[id];

            index++;
        }
    }
}


void ReconstructedImage::calculatePixelPointValueOnEquidistanceProjection(
        int ndiv, int imageWidth, int imageHeight,
        vector<double> values, vector<double> &rvalues)
{
    EquidistanceProjection edproj;
    EquisolidAngleProjection eaproj;

    edproj.setEquidistanceProjection(imageWidth, imageHeight);
    eaproj.setEquisolidAngleProjection(ndiv, ndiv);

    int index = 0;
    for(int ipy = 0; ipy < imageHeight; ipy++) {
        for(int ipx = 0; ipx < imageWidth; ipx++) {
            // ピクセル位置に対応する半球上の座標に変換する。
            //double[] spt = edproj.getHalfSphereCoordByIndex(ipx, ipy);
            vector<double> spt(3,0);
            bool bOuttheSphere = edproj.getHalfSphereCoordByIndex(spt, ipx, ipy, this->_cx, this->_cy);
            if(bOuttheSphere) {
                rvalues[index] = 0.0;
                index++;
                continue;
            }

            // 半球上の点を、等立体角座標における投影平面上の座標に変換する。
            //double[] ppt = CompCones.EquisolidAngleProjection.getProjectedPlaneCoordByHalfSpherePoint(spt);
            vector<double> ppt(2,0);
            eaproj.getProjectedPlaneCoordByHalfSpherePoint(ppt, spt);

            //int[] spidx = eaproj.getNearestIndexOnProjPlane(ppt[0], ppt[1]);      // ピクセルの4隅の点の全体インデックス
            vector<int> spidx(4,0);
            eaproj.getNearestIndexOnProjPlane(spidx, ppt[0], ppt[1], this->_cx, this->_cy);

            if(spidx[1] >= ndiv) { spidx[1] = ndiv - 1; }
            if(spidx[3] >= ndiv) { spidx[3] = ndiv - 1; }
            //
            if(spidx[0] >= ndiv) { spidx[0] = ndiv - 1; }
            if(spidx[2] >= ndiv) { spidx[2] = ndiv - 1; }

            //double[] ipt = eaproj.convertProjPlaneCoordToIndex(ppt[0], ppt[1]);   // XY方向のインデックス
            ////Console.WriteLine("" + ipx + "," + ipy + "," + spidx[0] + "," + spidx[1] + "," + spidx[2] + "," + spidx[3] + "," + ipt[0] + "," + ipt[1]);
            vector<double> ipt(2,0);
            eaproj.convertProjPlaneCoordToIndex(ppt[0], ppt[1], this->_cx, this->_cy);
            eaproj.getIXIY(ipt);

            int idx1 = spidx[2] * ndiv + spidx[0];
            int idx2 = spidx[2] * ndiv + spidx[1];
            int idx3 = spidx[3] * ndiv + spidx[0];
            int idx4 = spidx[3] * ndiv + spidx[1];

            double v1 = values[idx1];
            double v2 = values[idx2];
            double v3 = values[idx3];
            double v4 = values[idx4];

            // 内挿
            double vx1 = v1 * abs(spidx[1] - ipt[0]) + v2 * abs(spidx[0] - ipt[0]);
            double vx2 = v3 * abs(spidx[1] - ipt[0]) + v4 * abs(spidx[0] - ipt[0]);
            if(spidx[1] == spidx[0]) {
                vx1 = v1;
                vx2 = v3;
            }
            double vxy = vx1 * abs(spidx[3] - ipt[1]) + vx2 * abs(spidx[2] - ipt[1]);
            if(spidx[3] == spidx[2]) {
                vxy = vx1;
            }

            rvalues[index] = vxy;
            index++;
        }
    }
}


void ReconstructedImage::addScalerToImage(int width, int height, int imageRgb[][3])
{
    // 中央線(0度線)
    int cx = (int)round(width / 2.0);
    addScalerToImageHorizontal(width, height, imageRgb, cx, black);
    int cy = (int)round(height / 2.0);
    addScalerToImageVertical(width, height, imageRgb, cy, black);

    // -60,30,30,60度線
    cx = (int)round(width / 6.0);
    addScalerToImageHorizontal(width, height, imageRgb, cx, gray);
    cx = (int)round(width / 3.0);
    addScalerToImageHorizontal(width, height, imageRgb, cx, gray);
    cx = (int)round(width / 3.0 * 2.0);
    addScalerToImageHorizontal(width, height, imageRgb, cx, gray);
    cx = (int)round(width / 6.0 * 5.0);
    addScalerToImageHorizontal(width, height, imageRgb, cx, gray);

    cy = (int)round(height / 6.0);
    addScalerToImageVertical(width, height, imageRgb, cy, gray);
    cy = (int)round(height / 3.0);
    addScalerToImageVertical(width, height, imageRgb, cy, gray);
    cy = (int)round(height / 3.0 * 2.0);
    addScalerToImageVertical(width, height, imageRgb, cy, gray);
    cy = (int)round(height / 6.0 * 5.0);
    addScalerToImageVertical(width, height, imageRgb, cy, gray);

    if(width >= 200 && height >= 200) {
        // 10度ごとの線
        //double[] degree = new double[] {
        //        10.0  / 180.0,  20.0 / 180.0,
        //        40.0  / 180.0,  50.0 / 180.0,
        //        70.0  / 180.0,  80.0 / 180.0,
        //        100.0 / 180.0, 110.0 / 180.0,
        //        130.0 / 180.0, 140.0 / 180.0,
        //        160.0 / 180.0, 170.0 / 180.0,
        //};
        double degree[12] = {
                10.0 / 180.0,  20.0 / 180.0,
                40.0 / 180.0,  50.0 / 180.0,
                70.0 / 180.0,  80.0 / 180.0,
                100.0 / 180.0, 110.0 / 180.0,
                130.0 / 180.0, 140.0 / 180.0,
                160.0 / 180.0, 170.0 / 180.0,
        };

        //for(int i = 0; i < degree.Length; i++) {
        for(int i = 0; i < 12; i++) {
            cx = (int)round(width * degree[i]);
            addScalerToImageHorizontalTick(width, height, imageRgb, cx, llgray, 5);
        }
        //for(int i = 0; i < degree.Length; i++) {
        for(int i = 0; i < 12; i++) {
            cy = (int)round(height * degree[i]);
            addScalerToImageVerticalTick(width, height, imageRgb, cy, llgray, 5);
        }
    }
}


void ReconstructedImage::addScalerToImageHorizontal(int width, int height, int imageRgb[][3], int ix, Vector3 color)
{
    for(int ipy = 0; ipy < height; ipy++) {
        imageRgb[ipy * width + ix][0] = color[0];
        imageRgb[ipy * width + ix][1] = color[1];
        imageRgb[ipy * width + ix][2] = color[2];

        //cout << "RGB_BLACK " << imageRgb[ipy * width + ix][0] << endl;
        //cout << "RGB_BLACK " << imageRgb[ipy * width + ix][1] << endl;
        //cout << "RGB_BLACK " << imageRgb[ipy * width + ix][2] << endl;
        //system("pause");
    }
}


void ReconstructedImage::addScalerToImageVertical(int width, int height, int imageRgb[][3], int iy, Vector3 color)
{
    for(int ipx = 0; ipx < width; ipx++) {
        imageRgb[iy * width + ipx][0] = color[0];
        imageRgb[iy * width + ipx][1] = color[1];
        imageRgb[iy * width + ipx][2] = color[2];
    }
}


void ReconstructedImage::addScalerToImageHorizontalTick(int width, int height, int imageRgb[][3], int ix, Vector3 color, int length)
{
    for(int ipy = 0; ipy < length; ipy++) {
        imageRgb[ipy * width + ix][0] = color[0];
        imageRgb[ipy * width + ix][1] = color[1];
        imageRgb[ipy * width + ix][2] = color[2];
    }
    for(int ipy = height - length; ipy < height; ipy++) {
        imageRgb[ipy * width + ix][0] = color[0];
        imageRgb[ipy * width + ix][1] = color[1];
        imageRgb[ipy * width + ix][2] = color[2];
    }
}


void ReconstructedImage::addScalerToImageVerticalTick(int width, int height, int imageRgb[][3], int iy, Vector3 color, int length)
{
    for(int ipx = 0; ipx < length; ipx++) {
        imageRgb[iy * width + ipx][0] = color[0];
        imageRgb[iy * width + ipx][1] = color[1];
        imageRgb[iy * width + ipx][2] = color[2];
    }
    for(int ipx = width - length; ipx < width; ipx++) {
        imageRgb[iy * width + ipx][0] = color[0];
        imageRgb[iy * width + ipx][1] = color[1];
        imageRgb[iy * width + ipx][2] = color[2];
    }
}


void EquisolidAngleProjection::setEquisolidAngleProjection(int nx, int ny)
{
    this->_nx = nx;
    this->_ny = ny;
}


void EquisolidAngleProjection::getHalfSphereCoordByIndex(vector<double> &sph, int ix, int iy, double cx, double cy)
{
    if(ix < 0) { ix = 0; }
    if(ix > this->_nx) { ix = this->_nx; }
    if(iy < 0) { iy = 0; }
    if(iy > this->_ny) { iy = this->_ny; }

    //double rx = -1.0 + 2.0 * ix / _nx;
    //double ry = -1.0 + 2.0 * iy / _ny;
    double rx = -cx + (cx * 2.0) * ix / this->_nx;
    double ry = -cy + (cy * 2.0) * iy / this->_ny;
    double rxy = sqrt(rx * rx + ry * ry);

    if(rxy == 0.0) {
        sph[0] = 0.0;
        sph[1] = 0.0;
        sph[2] = cx;
        //return *sph;
    } else {
        double thetaxy = r2theta(rxy, cx);

        //return *sph;
        sph[0] = rx / rxy * sin(thetaxy);
        sph[1] = ry / rxy * sin(thetaxy);
        sph[2] = cos(thetaxy);
    }
}


void EquisolidAngleProjection::getProjectedPlaneCoordByIndex(vector<double> &rxy, int ix, int iy, double cx, double cy)
{
    if(ix < 0) { ix = 0; }
    if(ix > _nx) { ix = _nx; }
    if(iy < 0) { iy = 0; }
    if(iy > _ny) { iy = _ny; }

    //this->_rx = -1.0 + 2.0 * ix / this->_nx;
    //this->_ry = -1.0 + 2.0 * iy / this->_ny;
    rxy[0] = -cx + (cx * 2.0) * ix / _nx;
    rxy[1] = -cy + (cy * 2.0) * iy / _ny;
}


void EquisolidAngleProjection::getProjectedPlaneCoordByHalfSpherePoint(vector<double> &rxy, vector<double> sph)
{
    double theta = acos(sph[2]);
    double sind = sin(theta);
    double r = sqrt(2) * sin(theta * 0.5);

    double rx;
    double ry;

    if(sind == 0.0) {
        rx = 0.0;
        ry = 0.0;
        //return (rx, ry);
        rxy[0] = rx;
        rxy[1] = ry;
    } else {
        rxy[0] = r / sind * sph[0];
        rxy[1] = r / sind * sph[1];
    }

    //return (rx, ry);
    //return new double[] { rx, ry };
}


void EquisolidAngleProjection::convertProjPlaneCoordToIndex(double rx, double ry, double cx, double cy)
{
    double ix = (rx + cx) / (cx * 2.0) * this->_nx;
    double iy = (ry + cy) / (cy * 2.0) * this->_ny;

    // return new double[] { ix, iy };
    //return (ix, iy);
    this->_ix = ix;
    this->_iy = iy;
}


void EquisolidAngleProjection::getNearestIndexOnProjPlane(vector<int> &spidx, double rx, double ry, double cx, double cy)
{
    int ix1 = 0;
    int ix2 = 0;
    int iy1 = 0;
    int iy2 = 0;

    if(rx < -cx) {
        ix1 = 0;
        ix2 = 0;
    } else if(rx > cx) {
        ix1 = this->_nx;
        ix2 = this->_nx;
    } else {
        ix1 = 0;
        ix2 = 0;
        for(int ix = 0; ix < this->_nx; ix++) {
            //double x1 = -1.0 + 2.0 * (ix) / this->_nx;
            //double x2 = -1.0 + 2.0 * (ix + 1) / this->_nx;
            double x1 = -cx + (cx*2.0) * (ix) / this->_nx;
            double x2 = -cx + (cx*2.0) * (ix + 1) / this->_nx;
            if(rx >= x1 && rx <= x2) {
                ix1 = ix;
                ix2 = ix + 1;
            }
        }
    }

    if(ry < -cy) {
        iy1 = 0;
        iy2 = 0;
    } else if(ry > cy) {
        iy1 = this->_ny;
        iy2 = this->_ny;
    } else {
        iy1 = 0;
        iy2 = 0;
        for(int iy = 0; iy < this->_ny; iy++) {
            //double y1 = -1.0 + 2.0 * (iy) / this->_ny;
            //double y2 = -1.0 + 2.0 * (iy + 1) / this->_ny;
            double y1 = -cy + (cy*2.0) * (iy) / this->_ny;
            double y2 = -cy + (cy*2.0) * (iy + 1) / this->_ny;
            if(ry >= y1 && ry <= y2) {
                iy1 = iy;
                iy2 = iy1 + 1;
            }
        }
    }

    // return new int[] { ix1, ix2, iy1, iy2 };
    //return (ix1, ix2, iy1, iy2);
    spidx[0] = ix1;
    spidx[1] = ix2;
    spidx[2] = iy1;
    spidx[3] = iy2;
}


double EquisolidAngleProjection::theta2r(double theta, double cx)
{
    if(theta <= -HalfPI) { return -cx; }
    if(theta >= HalfPI) { return cx; }

    return Root2 * sin(theta * 0.5);
}


double EquisolidAngleProjection::r2theta(double r, double cx)
{
    if(r <= -cx) { return -M_PI / 2;}
    if(r >= cx) { return M_PI / 2; }

    return 2.0 * asin(r / sqrt(2));
}


double EquisolidAngleProjection::theta2sph(double theta)
{
    return sin(theta);
}


void EquisolidAngleProjection::getIXIY(vector<double> &ipt)
{
    ipt[0] = this->_ix;
    ipt[1] = this->_iy;
}
