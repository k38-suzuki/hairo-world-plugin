/**
   \file
   \author Kenta Suzuki
*/

#include "ImageGenerator.h"
#include <algorithm>
#include <cmath>
#include <random>
#include <vector>
#include <QColor>

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

}

namespace cnoid {

class ImageGeneratorImpl
{
public:
    ImageGeneratorImpl(ImageGenerator* self);

    ImageGenerator* self;
    random_device seed_gen;
    default_random_engine engine;
    normal_distribution<> dist;

    void barrelDistortion(Image& image, const double& m_coefb, const double& m_coefd);
    void gaussianNoise(Image& image, const double& m_std_dev);
    void hsv(Image& image, const double& m_hue, const double& m_saturation, const double& m_value);
    void rgb(Image& image, const double& m_red, const double& m_green, const double& m_blue);
    void saltPepperNoise(Image& image, const double& m_salt, const double& m_pepper);
    void filteredImage(Image& image, const double& m_scalex, const double& m_scaley);
    void flippedImage(Image& image);
    void gaussianFilter(Image& image, const int& matrix);
    void medianFilter(Image& image, const int& matrix);
    void sobelFilter(Image& image);
};

}


ImageGenerator::ImageGenerator()
{
    impl = new ImageGeneratorImpl(this);
}


ImageGeneratorImpl::ImageGeneratorImpl(ImageGenerator* self)
    : self(self)
{
    std::default_random_engine engineImpl(seed_gen());
    std::normal_distribution<> distImpl(0.0, 1.0);
    engine = engineImpl;
    dist = distImpl;
}


ImageGenerator::~ImageGenerator()
{
    delete impl;
}


void ImageGenerator::barrelDistortion(Image& image, const double& m_coefb, const double& m_coefd)
{
    impl->barrelDistortion(image, m_coefb, m_coefd);
}


void ImageGeneratorImpl::barrelDistortion(Image& image, const double& m_coefb, const double& m_coefd)
{
    Image cloneImage;
    cloneImage.setSize(image.width(), image.height(), image.numComponents());
    int width = image.width();
    int height = image.height();
    int nc = image.numComponents();

    double coefa = 0.0;
    double coefb = m_coefb;
    double coefc = 0.0;
    double coefd = m_coefd - coefa - coefb - coefc;

    for(int i = 0; i < width; i++) {
        for(int j = 0; j < height; j++) {
            int d = min(width, height) / 2;
            double cntx = (width - 1) / 2.0;
            double cnty = (height - 1) / 2.0;
            double delx = (i - cntx) / d;
            double dely = (j - cnty) / d;
            double dstr = sqrt(delx * delx + dely * dely);
            double srcr = (coefa * dstr * dstr * dstr + coefb * dstr * dstr + coefc * dstr + coefd) * dstr;
            double fctr = abs(dstr / srcr);
            double srcxd = cntx + (delx * fctr * d);
            double srcyd = cnty + (dely * fctr * d);
            int srcx = (int)srcxd;
            int srcy = (int)srcyd;
            if((srcx >= 0) && (srcy >= 0) && (srcx < width) && (srcy < height)) {
                int index =  nc * (j * width + i);
                for(int k = 0; k < nc; ++k) {
                    cloneImage.pixels()[index + k] = image.pixels()[nc * (srcy * width + srcx) + k];
                }
            }
        }
    }
    image = cloneImage;
}


void ImageGenerator::gaussianNoise(Image& image, const double& m_std_dev)
{
    impl->gaussianNoise(image, m_std_dev);
}


void ImageGeneratorImpl::gaussianNoise(Image& image, const double& m_std_dev)
{
    image.setSize(image.width(), image.height(), image.numComponents());
    int width = image.width();
    int height = image.height();
    int nc = image.numComponents();

    for(int j = 0; j < height; ++j) {
        for(int i = 0; i < width; ++i) {
            int index = nc * (i + j * width);
            double color = dist(engine) * m_std_dev * 255.0;
            for(int k = 0; k < nc; ++k) {
                int pixel = image.pixels()[index + k] + (int)color;
                if(pixel > 255) {
                    pixel = 255;
                } else if(pixel < 0) {
                    pixel = 0;
                }
                image.pixels()[index + k] = pixel;
            }
        }
    }
}


void ImageGenerator::hsv(Image& image, const double& m_hue, const double& m_saturation, const double& m_value)
{
    impl->hsv(image, m_hue, m_saturation, m_value);
}


void ImageGeneratorImpl::hsv(Image& image, const double& m_hue, const double& m_saturation, const double& m_value)
{
    image.setSize(image.width(), image.height(), image.numComponents());
    int width = image.width();
    int height = image.height();
    int nc = image.numComponents();

    for(int j = 0; j < height; ++j) {
        for(int i = 0; i < width; ++i) {
            int index = nc * (i + j * width);
            int colors[] = { 0, 0, 0 };
            for(int k = 0; k < nc; ++k) {
                colors[k] = (int)image.pixels()[index + k];
            }
            QColor rgbColor = QColor::fromRgb(colors[0], colors[1], colors[2]);
            int h = rgbColor.hue() + m_hue * 360.0;
            int s = rgbColor.saturation() + m_saturation * 255.0;
            int v = rgbColor.value() + m_value * 255.0;

            if(h > 359) {
                h -= 360;
            } else if(h < 0) {
                h = 0;
            }

            if(s > 255) {
                s = 255;
            } else if(s < 0) {
                s = 0;
            }

            if(v > 255) {
                v = 255;
            } else if(v < 0) {
                v = 0;
            }

            QColor hsvColor = QColor::fromHsv(h, s, v);
            int rgb[] = { hsvColor.red(), hsvColor.green(), hsvColor.blue() };
            for(int k = 0; k < nc; ++k) {
                image.pixels()[index + k] = rgb[k];
            }
        }
    }
}


void ImageGenerator::rgb(Image& image, const double& m_red, const double& m_green, const double& m_blue)
{
    impl->rgb(image, m_red, m_green, m_blue);
}


void ImageGeneratorImpl::rgb(Image& image, const double& m_red, const double& m_green, const double& m_blue)
{
    image.setSize(image.width(), image.height(), image.numComponents());
    int width = image.width();
    int height = image.height();
    int nc = image.numComponents();

    double colors[] = { m_red * 255.0,  m_green * 255.0, m_blue * 255.0};
    for(int j = 0; j < height; ++j) {
        for(int i = 0; i < width; ++i) {
            int index = nc * (i + j * width);
            for(int k = 0; k < nc; ++k) {
                int pixel = (double)image.pixels()[index + k] + colors[k];
                if(pixel > 255) {
                    pixel = 255;
                } else if(pixel < 0) {
                    pixel = 0;
                }
                image.pixels()[index + k] = pixel;
            }
        }
    }
}


void ImageGenerator::saltPepperNoise(Image& image, const double& m_salt, const double& m_pepper)
{
    impl->saltPepperNoise(image, m_salt, m_pepper);
}


void ImageGeneratorImpl::saltPepperNoise(Image& image, const double& m_salt, const double& m_pepper)
{
    image.setSize(image.width(), image.height(), image.numComponents());
    int width = image.width();
    int height = image.height();
    int nc = image.numComponents();

    for(int j = 0; j < height; ++j) {
        for(int i = 0; i < width; ++i) {
            int index = nc * (i + j * width);
            double salt = (double)(rand() % 101) / 100.0;
            double pepper = (double)(rand() % 101) / 100.0;
            for(int k = 0; k < nc; ++k) {
                if(salt < m_salt) {
                    image.pixels()[index + k] = 255;
                }
                if(pepper < m_pepper) {
                    image.pixels()[index + k] = 0;
                }
            }
        }
    }
}


void ImageGenerator::filteredImage(Image& image, const double& m_scalex, const double& m_scaley)
{
    impl->filteredImage(image, m_scalex, m_scaley);
}


void ImageGeneratorImpl::filteredImage(Image& image, const double& m_scalex, const double& m_scaley)
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
    impl->flippedImage(image);
}


void ImageGeneratorImpl::flippedImage(Image& image)
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
    impl->gaussianFilter(image, matrix);
}


void ImageGeneratorImpl::gaussianFilter(Image& image,  const int& matrix)
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
    impl->medianFilter(image, matrix);
}


void ImageGeneratorImpl::medianFilter(Image& image, const int& matrix)
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
    impl->sobelFilter(image);
}


void ImageGeneratorImpl::sobelFilter(Image& image)
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
        hkernel.push_back(sobel3[0][i]);
        vkernel.push_back(sobel3[1][i]);
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
                image.pixels()[index + k] = 0;
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
