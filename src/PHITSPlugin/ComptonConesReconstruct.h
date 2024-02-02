/**
   @author Kenta Suzuki
*/

#ifndef CNOID_PHITS_PLUGIN_COMPTON_CONE_RECONSTRUCT_H
#define CNOID_PHITS_PLUGIN_COMPTON_CONE_RECONSTRUCT_H

#include <cnoid/EigenTypes>
#include <vector>

namespace cnoid {

class EquisolidAngleProjection;

/// <summary>
/// コンプトンコーンの再構成を行う。
/// </summary>
class ComptonConesReconstruct
{
public:

    void setndiv(int ndiv, double theta);

    /// <summary>コンプトンコーン再構成のメイン。
    ///
    /// </summary>
    /// <param name="infilepath">コンプトンコーンデータファイル</param>
    /// <param name="respfilepath">角度感度補正関数のファイル</param>
    /// <param name="sphere_radius">半球の半径 [mm]</param>
    /// <param name="arm">ARM角度 (度)</param>
    /// <param name="ndiv">分割数</param>
    /// <param name="camera_width">カメラの幅 [mm]</param>
    /// <param name="camera_height">カメラの高さ [mm]</param>
    /// <returns></returns>
    void Exec(std::vector<double> &values, double camera_width, double camera_height, double sphere_radius,
              double arm, int cnt, std::vector<double> x1, std::vector<double> z1, std::vector<double> x2,
              std::vector<double> z2, double Ga, std::vector<double> th, std::vector<int> iflg);

private:
    int _NumCones;
    int _ndiv;
    int _nx;
    int _ny;
    double _cx;
    double _cy;
};


class ReconstructedImage
{
public:
    ReconstructedImage();
    virtual ~ReconstructedImage();

    void SetImageSize(int ndiv, int imageSize, double theta);

    /// <summary>
    ///
    /// </summary>
    /// <param name="imageSize"></param>
    /// <param name="projectionTypeIndex"></param>
    /// <param name="reconstValue"></param>
    /// <param name="displayRegionCoeffX"></param>
    /// <param name="displayRegionCoeffY"></param>
    /// <returns></returns>
    //public: int[][] CreateImage(
    void CreateImage(std::vector<std::vector<int>> &imageRgb, std::vector<double> values,
        int projectionTypeIndex, double displayRegionCoeffX, double displayRegionCoeffY);

    /// <summary>等立体角射影で、画像のピクセル点における値を算出する。
    ///
    /// </summary>
    /// <param name="ndiv"></param>
    /// <param name="imageWidth"></param>
    /// <param name="imageHeight"></param>
    /// <param name="reconstValue"></param>
    /// <param name="displayRegionCoeffX"></param>
    /// <param name="displayRegionCoeffY"></param>
    /// <param name="rvalues"></param>
    void calculatePixelPointValueOnEquisolidAngleProjection(
            int ndiv, int imageWidth, int imageHeight, std::vector<double> values,
            double displayRegionCoeffX, double displayRegionCoeffY, std::vector<double> &rvalues);


    /// <summary>等距離射影で、画像のピクセル点における値を算出する。
    ///
    /// </summary>
    /// <param name="ndiv"></param>
    /// <param name="imageWidth"></param>
    /// <param name="imageHeight"></param>
    /// <param name="reconstValue"></param>
    /// <param name="rvalues"></param>
    void calculatePixelPointValueOnEquidistanceProjection(
            int ndiv, int imageWidth, int imageHeight,
            std::vector<double> values, std::vector<double> &rvalues);


    /// <summary>画像にメモリを追加する。
    /// 縦横ともに-90度から90度である。
    /// 0度線(黒)と30度ごとの線(灰色)、画像サイズが200ピクセル以上のときは10度ごとのチック線を描く。
    ///
    /// imageRgb[0] は画像左上のピクセルのRGB。
    /// </summary>
    /// <param name="width"></param>
    /// <param name="height"></param>
    /// <param name="imageRgb"></param>
    void addScalerToImage(int width, int height, int imageRgb[][3]);
    void addScalerToImageHorizontal(int width, int height, int imageRgb[][3], int ix, Vector3 color);
    void addScalerToImageVertical(int width, int height, int imageRgb[][3], int iy, Vector3 color);
    void addScalerToImageHorizontalTick(int width, int height, int imageRgb[][3], int ix, Vector3 color, int length);
    void addScalerToImageVerticalTick(int width, int height, int imageRgb[][3], int iy, Vector3 color, int length);

private:
    Vector3 white;
    Vector3 black;
    Vector3 dgray;
    Vector3 gray;
    Vector3 lgray;
    Vector3 llgray;

    int _ndiv;
    int _imageWidth;
    int _imageHeight;
    double _cx;
    double _cy;
};

}

#endif
