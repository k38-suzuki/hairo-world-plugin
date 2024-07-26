/**
   @author Kenta Suzuki
*/

#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "ComptonCone.h"
#include <cnoid/BodyItem>
#include <cnoid/DeviceList>
#include <cnoid/Format>
#include <cnoid/ItemList>
#include <cnoid/RootItem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <sstream>
#include <vector>
#include "ComptonConesReconstruct.h"
#include "stb_image_write.h"
#include "gettext.h"

using namespace std;
using namespace cnoid;

namespace {

struct RGBA {
    unsigned char r, g, b, a; // red, green, blue, alpha
    RGBA() = default;
    constexpr RGBA(const unsigned char r_, const unsigned char g_, const unsigned char b_, const unsigned char a_) :r(r_), g(g_), b(b_), a(a_) {}
    constexpr bool operator&&(const RGBA& rgba_) noexcept {
        return this->r == rgba_.r && this->g == rgba_.g && this->b == rgba_.b && this->a == rgba_.a;
    }
};

void ConvertToBitmapSource(const char *pngfile, int iSize, vector<vector<int> > rgb)
{
    const int imageSize = 100;

    constexpr size_t width{ imageSize }, height{ imageSize };

    unique_ptr<RGBA[][width]> rgba(new(nothrow) RGBA[height][width]);
    //if(!rgba) exit(1) ;

    for(int ipy = 0; ipy < imageSize; ipy++) {
        for(int ipx = 0; ipx < imageSize; ipx++) {
            rgba[ipy][ipx].r = rgb[ipy * imageSize + ipx][0];
            rgba[ipy][ipx].g = rgb[ipy * imageSize + ipx][1];
            rgba[ipy][ipx].b = rgb[ipy * imageSize + ipx][2];
            rgba[ipy][ipx].a = 255;
        }
    }

    stbi_write_png(pngfile, static_cast<int>(width), static_cast<int>(height), static_cast<int>(sizeof(RGBA)), rgba.get(), 0);
}

string replaceAll(string &replacedStr, string from, string to) {
    unsigned int pos = replacedStr.find(from);
    int toLen = to.length();

    if(from.empty()) {
        return replacedStr;
    }

    //while((pos = replacedStr.find(from, pos)) != string::npos) {
    while((pos = replacedStr.find(from, pos)) != -1) {
        replacedStr.replace(pos, from.length(), to);
        pos += toLen;
    }
    return replacedStr;
}

double norm2(double *v, int length)
{
    double sum = 0.0;
    //double length = sizeof(*v) / sizeof(v) + 1.0;
    //cout << " len  " << length << " " << sizeof(v)  << " " << sizeof(*v) << endl;

    //for(int i = 0, n = v.length; i < n; i++) {
    for(int i = 0, n = length; i < n; i++) {
        sum = sum + v[i] * v[i];
    }

    //cout << " sum  " << sum << endl;
    return sqrt(sum);
}

double getT0ofPointToLine(vector<double> a, vector<double> v, vector<double> b)
{
    return (v[0] * (b[0] - a[0]) + v[1] * (b[1] - a[1]) + v[2] * (b[2] - a[2]));
}

double getLengthOfPerpendicular(vector<double> a, vector<double> v, vector<double> b, double t0)
{
    double d0 = a[0] + t0 * v[0] - b[0];
    double d1 = a[1] + t0 * v[1] - b[1];
    double d2 = a[2] + t0 * v[2] - b[2];

    return sqrt(d0 * d0 + d1 * d1 + d2 * d2);
}

class ReconstructedConesIO
{
public:

    void SaveAsText(string outfilepath, double radius, double arm, int ndiv, int numCones, double *values)
    {
        //auto enc = new UTF8Encoding(false);
        //using (StreamWriter writer = new StreamWriter(outfilepath, false, enc)) {
        //    writer.WriteLine(radius);
        //    writer.WriteLine(arm);
        //    writer.WriteLine(numCones);
        //    writer.WriteLine(ndiv);
        //    for(int i = 0; i < values.Length; i++) {
        //        writer.Write(String.Format("{0,10:0.000E+00}", values[i]));
        //        if((i + 1) % 10 == 0) {
        //            writer.WriteLine("");
        //        } else {
        //            if(i != (values.Length - 1)) {
        //                writer.Write(",");
        //            }
        //        }
        //    }

        //    if(values.Length % 10 != 0) {
        //        writer.WriteLine("");
        //    }
        //};

        int length = ndiv * ndiv;

        //const char *outfilepath = "recon_test.txt";
        ofstream writing_file;
        writing_file.open(outfilepath, ios::out);

        writing_file << radius << endl;
        writing_file << arm << endl;
        writing_file << numCones << endl;
        writing_file << ndiv << endl;

        for(int i = 0; i < length; i++) {
            writing_file << scientific << setprecision(3) << uppercase << values[i];
            if((i + 1) % 100 == 0 || i == (length - 1)) {
                writing_file << "" << endl;
            } else {
                if(i != (length - 1)) {
                    writing_file << ",";
                }
            }
        }
    }

    void SaveAsTmp(string outfilepath, int ndiv, vector<double> values, double radius, int imageSize)
    {
        int length = ndiv * ndiv;

        ofstream writing_file;
        writing_file.open(outfilepath, ios::out);
        writing_file << fixed;
        writing_file << "[ T - C r o s s ]" << endl;
        writing_file << "    title = [t-cross] in xyz mesh" << endl;
        writing_file << "     mesh =  xyz            # mesh type is xyz scoring mesh" << endl;
        writing_file << "   x-type =    2            # x-mesh is linear given by xmin, xmax and nx" << endl;
        writing_file << "     xmin =  -100.0000      # minimum value of x-mesh points" << endl;
        writing_file << "     xmax =   100.0000      # maximum value of x-mesh points" << endl;
        writing_file << "       nx =    100          # number of x-mesh points" << endl;
        writing_file << "   y-type =    2            # y-mesh is linear given by ymin, ymax and ny" << endl;
        writing_file << "     ymin =  -1.000000      # minimum value of y-mesh points" << endl;
        writing_file << "     ymax =   1.000000      # maximum value of y-mesh points" << endl;
        writing_file << "       ny =      1          # number of y-mesh points" << endl;
        writing_file << "   z-type =    2            # z-mesh is linear given by zmin, zmax and nz" << endl;
        writing_file << "     zmin =  -100.0000      # minimum value of z-mesh points" << endl;
        writing_file << "     zmax =   100.0000      # maximum value of z-mesh points" << endl;
        writing_file << "       nz =   100           # number of z-mesh points" << endl;
        writing_file << "   e-type =    2            # e-mesh is linear given by emin, emax and ne" << endl;
        writing_file << "     emin =   0.000000      # minimum value of e-mesh points" << endl;
        writing_file << "     emax =   3.000000      # maximum value of e-mesh points" << endl;
        writing_file << "       ne =      1          # number of e-mesh points" << endl;
        writing_file << "" << endl;
        writing_file << "x: x [cm]" << endl;
        writing_file << "y: z [cm]" << endl;
        writing_file << "" << endl;
        writing_file << "hc:  y =  -99.00000     to   99.00000     by   2.000000     ; x =  -99.00000     to   99.00000     by   2.000000     ;" << endl;

        for(int i = 0; i < length; i++) {
            writing_file << scientific << setprecision(3) << uppercase << values[i];
            if((i + 1) % 10 == 0 || i == (length - 1)) {
                writing_file << "" << endl;
            } else {
                if(i != (length - 1)) {
                    writing_file << "  ";
                }
            }
        }
    }

private:
    const char SPLITTER = ',';
};

}


ComptonCone::ComptonCone()
{

}


ComptonCone::~ComptonCone()
{

}


bool ComptonCone::readComptonCone(string strFName, double Energy, ComptonCamera* camera)
{
    int numCones;

    double kf;
    double u, v, w;
    double e1, e2;
    double c1, c2, c3;
    double coef;

    Vector2 resolution = Vector2(8, 8);
    double det_elementWidth;
    double det_scattererThickness;
    double det_distance;
    double det_arm;
    double det_angle;

    det_elementWidth = camera->elementWidth();
    det_scattererThickness = camera->scattererThickness();
    det_distance = camera->distance();
    det_arm = camera->arm();
    resolution = camera->resolution();
    det_angle = (camera->fieldOfView()) * 180 / M_PI;

    const double e = Energy;
    const double mec2 = 0.51048;
    const double ang = 90.0;
    const double elementDistance = 0.06;

    const double ScatY = -det_scattererThickness;
    const double Gapsa = det_distance * 10.0;

    const double cameraWidth = ((det_elementWidth + elementDistance * 2) * resolution[0] - elementDistance * 2) * 10;
    const double cameraHeight = ((det_elementWidth + elementDistance * 2) * resolution[0]- elementDistance * 2) * 10;
    const double sphere_radius = 1000.0;
    const double arm = det_arm;
    const double displayRegionCoeffX = 1.0;
    const double displayRegionCoeffY = 1.0;

    const int ndiv = 100;
    const int nx = ndiv - 1;
    const int ny = ndiv - 1;
    const int nxy = (nx + 1) * (ny + 1);
    const int imageSize = 100;
    const int projectionTypeIndex = 1;

    string strCSVName = strFName;
    string strTMPName = strFName;
    string strPNGName = strFName;

    //const char *dmpfile = "flux_cross_dmp.out";
    //const char *outfile = "test.csv";
    const char *dmpfile = strFName.c_str();

    strCSVName = strFName + ".csv";
    const char *outfile = strCSVName.c_str();

    strTMPName = strFName + ".tmp";
    const char *tmpfile = strTMPName.c_str();

    strPNGName = strFName + "_CompCone.png";
    const char *pngfile = strPNGName.c_str();

    ifstream ifs;

    bool bDumpPID = false;
    string strPIDName = strFName + ".001" ;
    const char *dmpfilePID = strPIDName.c_str();
    ifs.open(dmpfilePID, ios::in);
    if(ifs) { bDumpPID = true; }
    ifs.close();

    bool bDump = false;
    ifs.open(dmpfile, ios::in);
    if(ifs) { bDump = true; }
    ifs.close();

    if(bDumpPID == false && bDump == false) {
        cout << "Dump file is not found." << endl;
        cin.get();
        return false;
    }

    ofstream writing_file;
    writing_file.open(outfile, ios::out);
    if(!writing_file) {
        cout << "Csv file is not found." << endl;
        cin.get();
        return false;
    }
    writing_file << "散乱X(mm), 散乱Y(mm), 吸収X(mm), 吸収Y(mm), 散乱 - 吸収間距離(mm), 角度(θ)" << endl;

    int ncnt = 0;
    int cnt = 0;
    numCones = 0;

    vector<int> iflg;
    vector<double> x1;
    vector<double> y1;
    vector<double> z1;
    vector<double> x2;
    vector<double> y2;
    vector<double> z2;
    vector<double> th;

    int pid = 0;

    while(1) {
        if(bDumpPID) {
            pid++;
            string strPIDName = strFName + "." + formatC("{:03d}", pid);
            const char *dmpfilePID = strPIDName.c_str();
            ifs.open(dmpfilePID, ios::in);
            if(!ifs) break;
        } else {
            ifs.open(dmpfile, ios::in);
        }

        string str;
        while(getline(ifs, str)) {
            iflg.push_back(0);
            x1.push_back(0);
            y1.push_back(0);
            z1.push_back(0);
            x2.push_back(0);
            y2.push_back(0);
            z2.push_back(0);
            th.push_back(0);

            replaceAll(str, "D", "E");
            istringstream stream(str);
            stream >> kf >> x2[cnt] >> y2[cnt] >> z2[cnt] >> u >> v >> w >> e2 >> c1 >> c2 >> c3;

            coef = (y2[cnt] - ScatY) / v;

            x1[cnt] = (x2[cnt] - u * coef) * 10;
            y1[cnt] = (y2[cnt] - v * coef) * 10;
            z1[cnt] = (z2[cnt] - w * coef) * 10;
            x2[cnt] = x2[cnt] * 10;
            y2[cnt] = y2[cnt] * 10;
            z2[cnt] = z2[cnt] * 10;
            e1 = e - e2;

            th[cnt] = 1 - mec2 * (1 / e2 - 1 / (e2 + e1));
            th[cnt] = acos(th[cnt]) * 180 / M_PI;
            th[cnt] = ang - th[cnt];
            th[cnt] = cos(th[cnt] / 180 * M_PI);

            x1[cnt] = round(x1[cnt] * 100) / 100;
            z1[cnt] = round(z1[cnt] * 100) / 100;
            x2[cnt] = round(x2[cnt] * 100) / 100;
            z2[cnt] = round(z2[cnt] * 100) / 100;
            th[cnt] = round(th[cnt] * 100) / 100;

            if((x1[cnt] >= -cameraWidth / 2 && x1[cnt] <= cameraWidth / 2) &&
                (z1[cnt] >= -cameraHeight / 2 && z1[cnt] <= cameraHeight / 2) &&
                e1 != 0.0 &&
                th[cnt] > 0.0) {
                iflg[cnt] = 1;
                numCones++;

                writing_file << fixed;
                writing_file << setprecision(2) << x1[cnt] << "," << z1[cnt] << "," << x2[cnt] << "," << z2[cnt] << "," << Gapsa << "," << th[cnt] << endl;
            } else {
                iflg[cnt] = 0;
            }
            cnt++;
        }

        ifs.close();
        if(!bDumpPID) { break; }
    
    }

    writing_file.close();

    ComptonConesReconstruct recon;
    ReconstructedConesIO reconstio;
    ReconstructedImage reconstimage;

    vector<double> values(nxy, 0);

    vector<vector<int>> imageRgb(imageSize*imageSize, vector<int>(3,0));
    double theta = (det_angle / 2.0) / ang;

    recon.setndiv(ndiv, theta);
    recon.Exec(values,
                cameraWidth, 
                cameraHeight,
                sphere_radius,
                arm,
                cnt, x1, z1, x2, z2, Gapsa, th, iflg);
    //reconstio->SaveAsText(txtfile, sphere_radius, arm, ndiv, numCones, values);
    reconstio.SaveAsTmp(tmpfile, ndiv, values, sphere_radius, imageSize);

    reconstimage.SetImageSize(ndiv, imageSize, theta);
    reconstimage.CreateImage(imageRgb, values, projectionTypeIndex, displayRegionCoeffX, displayRegionCoeffY);
    //reconstimage->addScalerToImage(imageSize, imageSize, imageRgb);

    ConvertToBitmapSource(pngfile, imageSize, imageRgb);
    
    return true;
}


void ComptonCone::setPosition(double ScatterX, double ScatterY)
{
    this->_Position.resize(3, 0);
    this->_Position[0] = ScatterX;
    this->_Position[1] = ScatterY;
    this->_Position[2] = 0.0;
    //cout << " get_Position  " << Position[0] << " " << Position[1] << " " << Position[2] << endl;
}


void ComptonCone::setDirection(int index, double ScatterX, double ScatterY, double AbsorbX, double AbsorbY, double Gapsa)
{
    double vas[3];

    vas[0] = ScatterX - AbsorbX;
    vas[1] = ScatterY - AbsorbY;
    vas[2] = Gapsa;
    double len_as = norm2(vas, 3);

    //cout << " vas[0]  " << vas[0] << " " << endl;
    //cout << " vas[1]  " << vas[1] << " " << endl;
    //cout << " vas[2]  " << vas[2] << " " << endl;
    //cout << " len_as  " << len_as << endl;

    if(len_as <= 0.0) {
        //throw new NotFiniteNumberException("len_as is zero.");
        throw "len_as is zero.";
    }
    this->_Direction.resize(3, 0);
    this->_Direction[0] = vas[0] / len_as;
    this->_Direction[1] = vas[1] / len_as;
    this->_Direction[2] = vas[2] / len_as;
}


// Returns whether the specified point is contained within the specified ARM of this cone.
bool ComptonCone::isPointContainedInArm(vector<double> point, double arm)
{
    //cout << " Position  " << Position[0] << " " << Position[1] << " " << Position[2] << endl;
    //cout << " Direction " << Direction[0] << " " << Direction[1] << " " << Direction[2] << endl;
    //cout << " point     " << point[0] << " " << point[1] << " " << point[2] << endl;
    //cout << " HAngle    " << HAngle << endl;
    //cout << " arm       " << arm << endl;

    double t0 = getT0ofPointToLine(this->_Position, this->_Direction, point);
    double d = getLengthOfPerpendicular(this->_Position, this->_Direction, point, t0);

    //cout << " t0 " << t0 << endl;
    if(t0 < 0.0) { return false; }

    double theta = atan(d / t0);
    theta = theta / M_PI * 180.0;        // radian -> degree

    //cout << " theta        " << theta << endl;
    //cout << " HAngle       " << HAngle << endl;
    //cout << " HAngle - arm " << HAngle - arm << endl;
    //cout << " HAngle + arm " << HAngle + arm << endl;

    if((theta >= HAngle - arm) && (theta <= HAngle + arm)) {
        return true;
    }

    return false;
}
