/**
   @author Kenta Suzuki
*/

#include "ImageGenerator.h"
#include <cnoid/MathUtil>
#include <vector>

using namespace std;
using namespace cnoid;

namespace {

const double gaussian3[] = { 1.0 / 16.0, 2.0 / 16.0, 1.0 / 16.0,
                             2.0 / 16.0, 4.0 / 16.0, 2.0 / 16.0,
                             1.0 / 16.0, 2.0 / 16.0, 1.0 / 16.0 };

const double gaussian5[] = { 1.0 / 256.0, 4.0 / 256.0, 6.0 / 256.0, 4.0 / 256.0, 1.0 / 256.0,
                             4.0 / 256.0, 16.0 / 256.0, 24.0 / 256.0, 16.0 / 256.0, 4.0 / 256.0,
                             6.0 / 256.0, 24.0 / 256.0, 36.0 / 256.0, 24.0 / 256.0, 6.0 / 256.0,
                             4.0 / 256.0, 16.0 / 256.0, 24.0 / 256.0, 16.0 / 256.0, 4.0 / 256.0,
                             1.0 / 256.0, 4.0 / 256.0, 6.0 / 256.0, 4.0 / 256.0, 1.0 / 256.0 };

const double sobel3[2][9] = { { -1.0, 0.0, 1.0, -2.0, 0.0, 2.0, -1.0, 0.0, 1.0 },
                              { 1.0, 2.0, 1.0, 0.0, 0.0, 0.0, -1.0, -2.0, -1.0 } }; // [0]-horizontal, [1]-vertical

const double prewitt3[2][9] = { { -1.0, 0.0, 1.0, -1.0, 0.0, 1.0, -1.0, 0.0, 1.0 },
                                { 1.0, 1.0, 1.0, 0.0, 0.0, 0.0, -1.0, -1.0, -1.0 } }; // [0]-horizontal, [1]-vertical

}

namespace cnoid {

class ImageGenerator::Impl
{
public:

    Impl();

    void differentialFilter(Image& image, const double kernel[2][9]);
};

}


ImageGenerator::ImageGenerator()
{
    impl = new Impl;
}


ImageGenerator::Impl::Impl()
{

}


ImageGenerator::~ImageGenerator()
{
    delete impl;
}


void ImageGenerator::filteredImage(Image& image, const double& m_scalex, const double& m_scaley)
{
    //Linear interpolation
    Image cloneImage = image;
    cloneImage.setSize(image.width(), image.height(), image.numComponents());
    int width = image.width();
    int height = image.height();
    int nc = image.numComponents();
    double scalex = m_scalex;
    double scaley = m_scaley;
    int xs = width / 2;
    int ys = height / 2;

    for(int i = -ys; i < ys; i++) {
        for(int j = -xs; j < xs; j++) {
            int y = i / scaley;
            int x = j / scalex;
            int m, n;
            if(y > 0) {
                m = (int)y;
            } else {
                m = (int)(y - 1);
            }
            if(x > 0) {
                n = (int)x;
            } else {
                n = (int)(x - 1);
            }
            int q = y - m;
            int p = x - n;
            if(q == 1) {
                q = 0;
                m++;
            }
            if(p == 1) {
                p = 0;
                n++;
            }

            for(int k = 0; k < nc; k++) {
                int d;
                if((m > -ys) && (m < ys) && (n >= -xs) && (n < xs)) {
                    d = (int)((1.0 - q) * ((1.0 - p) * image.pixels()[3 * ((m  + ys) * width + (n  + xs)) + k]
                            + p * image.pixels()[3 * ((m  + ys) * width + (n + 1 + xs)) + k])
                            + q * ((1.0 - p) * image.pixels()[3 * ((m + 1 + ys) * width + (n + xs)) + k]
                            + p * image.pixels()[3 * ((m + 1 + ys) * width + (n + 1 + xs)) + k]));
                } else {
                    d = 0;
                }
                if(d < 0) {
                    d = 0;
                }
                if(d > 255) {
                    d = 255;
                }
                cloneImage.pixels()[3 * ((i + ys) * width + (j + xs)) + k] = d;
            }
        }
    }
    image = cloneImage;
}


void ImageGenerator::flippedImage(Image& image)
{
    Image cloneImage = image;
    cloneImage.setSize(image.width(), image.height(), image.numComponents());
    int width = image.width();
    int height = image.height();
    int nc = image.numComponents();

    for(int j = 0; j < height; ++j) {
        for(int i = 0; i < width; ++i) {
            int index = nc * (i + j * width);
            for(int k = 0; k < nc; ++k) {
                cloneImage.pixels()[index + k] = image.pixels()[nc * ((width - 1 - i) + (height - 1 - j) * width) + k];
            }
        }
    }
    image = cloneImage;
}


void ImageGenerator::gaussianFilter(Image& image, const int& matrix)
{
    Image cloneImage;
    int width = image.width();
    int height = image.height();
    int nc = image.numComponents();
    int margin = matrix - 1;
    cloneImage.setSize(width + margin, height + margin, nc);

    vector<double> kernel;
    for(int i = 0; i < matrix * matrix; ++i) {
        if(matrix == 3) {
            kernel.push_back(gaussian3[i]);
        } else if(matrix == 5) {
            kernel.push_back(gaussian5[i]);
        } else {
            kernel.push_back(0.0);
        }
    }

    for(int j = 0; j < height; ++j) {
        for(int i = 0; i < width; ++i) {
            int index = nc * ((i + 1) + (j + 1) * (width + margin));
            for(int k = 0; k < nc; ++k) {
                cloneImage.pixels()[index + k] = image.pixels()[nc * (i + j * width) + k];
            }
        }
    }

    for(int j = 0; j < height; ++j) {
        for(int i = 0; i < width; ++i) {
            int index = nc * (i + j * width);
            for(int k = 0; k < nc; ++k) {
                image.pixels()[index + k] = 0;
                for(int p = j; p < j + matrix; ++p) {
                    for(int q = i; q < i + matrix; ++q) {
                        int r = p - j;
                        int s = q - i;
                        int t = matrix * r + s;
                        image.pixels()[index + k] += kernel[t] * (double)cloneImage.pixels()[nc * (q + p * (width + margin)) + k];
                    }
                }
            }
        }
    }
}


void ImageGenerator::medianFilter(Image& image, const int& matrix)
{
    Image cloneImage;
    int width = image.width();
    int height = image.height();
    int nc = image.numComponents();
    int margin = matrix - 1;
    int median = (matrix * matrix + 1) / 2;
    cloneImage.setSize(width + margin, height + margin, nc);

    for(int j = 0; j < height; ++j) {
        for(int i = 0; i < width; ++i) {
            int index = nc * ((i + 1) + (j + 1) * (width + margin));
            for(int k = 0; k < nc; ++k) {
                cloneImage.pixels()[index + k] = image.pixels()[nc * (i + j * width) + k];
            }
        }
    }

    for(int j = 0; j < height; ++j) {
        for(int i = 0; i < width; ++i) {
            int index = nc * (i + j * width);
            for(int k = 0; k < nc; ++k) {
                vector<double> values;
                for(int p = j; p < j + matrix; ++p) {
                    for(int q = i; q < i + matrix; ++q) {
                        values.push_back((double)cloneImage.pixels()[nc * (q + p * (width + margin)) + k]);
                    }
                }
                sort(values.begin(), values.end());
                image.pixels()[index + k] = values[median];
            }
        }
    }
}


void ImageGenerator::sobelFilter(Image& image)
{
    impl->differentialFilter(image, sobel3);
}


void ImageGenerator::prewittFilter(Image& image)
{
    impl->differentialFilter(image, prewitt3);
}


void ImageGenerator::Impl::differentialFilter(Image& image, const double kernel[2][9])
{
    Image cloneImage;
    int width = image.width();
    int height = image.height();
    int nc = image.numComponents();
    int matrix = 3;
    int margin = matrix - 1;
    cloneImage.setSize(width + margin, height + margin, nc);

    vector<double> hkernel;
    vector<double> vkernel;
    for(int i = 0; i < matrix * matrix; ++i) {
        hkernel.push_back(kernel[0][i]);
        vkernel.push_back(kernel[1][i]);
    }

    for(int j = 0; j < height; ++j) {
        for(int i = 0; i < width; ++i) {
            int index = nc * ((i + 1) + (j + 1) * (width + margin));
            for(int k = 0; k < nc; ++k) {
                cloneImage.pixels()[index + k] = image.pixels()[nc * (i + j * width) + k];
            }
        }
    }

    for(int j = 0; j < height; ++j) {
        for(int i = 0; i < width; ++i) {
            int index = nc * (i + j * width);
            for(int k = 0; k < nc; ++k) {
                image.pixels()[index + k] = 0;
                for(int p = j; p < j + matrix; ++p) {
                    for(int q = i; q < i + matrix; ++q) {
                        int r = p - j;
                        int s = q - i;
                        int t = matrix * r + s;
                        image.pixels()[index + k] += hkernel[t] * (double)cloneImage.pixels()[nc * (q + p * (width + margin)) + k];
                    }
                }
            }
        }
    }

    for(int i = 0; i < width; ++i) {
        for(int j = 0; j < height; ++j) {
            int index = nc * (i + j * width);
            for(int k = 0; k < nc; ++k) {
                for(int p = j; p < j + matrix; ++p) {
                    for(int q = i; q < i + matrix; ++q) {
                        int r = p - j;
                        int s = q - i;
                        int t = matrix * r + s;
                        image.pixels()[index + k] += vkernel[t] * (double)cloneImage.pixels()[nc * (q + p * (width + margin)) + k];
                    }
                }
            }
        }
    }
}


Image toCnoidImage(const QImage& image)
{
    Image cimage;
    cimage.setSize(image.width(), image.height(), 3);
    unsigned char* pixels = cimage.pixels();
    for(int j = 0 ; j < image.height() ; ++j) {
        for(int i = 0 ; i < image.width() ; ++i) {
            int index = (i + j * image.width()) * 3;
            QRgb rgb = image.pixel(i, j);
            pixels[index] = qRed(rgb);
            pixels[index + 1] = qGreen(rgb);
            pixels[index + 2] = qBlue(rgb);
        }
    }
    return cimage;
}


QImage toQImage(const Image& image)
{
    QImage qimage(image.width(), image.height(), QImage::Format_RGB888);
    const unsigned char* pixels = image.pixels();
    for(int j = 0 ; j < image.height() ; ++j) {
        for(int i = 0 ; i < image.width() ; ++i) {
            int index = (i + j * image.width()) * 3;
            QColor color(pixels[index], pixels[index + 1], pixels[index + 2]);
            qimage.setPixelColor(i, j, color);
        }
    }
    return qimage;
}
