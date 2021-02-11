/**
   \file
   \author Kenta Suzuki
*/

#include "ImageGenerator.h"
#include <cmath>
#include <algorithm>
#include <random>
#include <QColor>

using namespace std;
using namespace cnoid;

namespace cnoid {

class ImageGeneratorImpl
{
public:
    ImageGeneratorImpl(ImageGenerator* self);

    ImageGenerator* self;
    random_device seed_gen;
    default_random_engine engine;
    normal_distribution<> dist;

    Image barrelDistortion(const Image image, const double m_coefb, double m_coefd);
    Image gaussianNoise(const Image image, const double m_std_dev);
    Image hsv(const Image image, const double m_hue, const double m_saturation, const double m_value);
    Image rgb(const Image image, const double m_red, const double m_green, const double m_blue);
    Image saltPepperNoise(const Image image, const double m_salt, const double m_pepper);
    Image filteredImage(const Image image, const double m_scalex, const double m_scaley);
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


Image ImageGenerator::barrelDistortion(const Image image, const double m_coefb, double m_coefd)
{
    return impl->barrelDistortion(image, m_coefb, m_coefd);
}


Image ImageGeneratorImpl::barrelDistortion(const Image image, const double m_coefb, double m_coefd)
{
    Image cloneImage;
    cloneImage.setSize(image.width(), image.height(), image.numComponents());
    int width = image.width();
    int height = image.height();
    int numComp = image.numComponents();
    int length = width * height * numComp;

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
                int index = j * width + i;
                if(numComp == 3) {
                    for(int k = 0; k < 3; k++) {
                        cloneImage.pixels()[3 * index + k] = image.pixels()[3 * (srcy * width + srcx) + k];
                    }
                } else if(numComp == 1) {
                    cloneImage.pixels()[3 * index] = image.pixels()[3 * (srcy * width + srcx)];
                }
            }
        }
    }
    return cloneImage;
}


Image ImageGenerator::gaussianNoise(const Image image, const double m_std_dev)
{
    return impl->gaussianNoise(image, m_std_dev);
}


Image ImageGeneratorImpl::gaussianNoise(const Image image, const double m_std_dev)
{
    Image cloneImage = image;
    cloneImage.setSize(image.width(), image.height(), image.numComponents());
    int width = image.width();
    int height = image.height();
    int numComp = image.numComponents();
    int length = width * height * numComp;

    if(numComp == 3) {
        for(int i = 0; i < length; i += 3) {
            double addLumi = dist(engine) * m_std_dev * 255.0;
            int pixel[3];

            for(int j = 0; j < 3; j++) {
                pixel[j] = image.pixels()[i + j] + (int)addLumi;
                if(pixel[j] > 255) {
                    pixel[j] = 255;
                } else if(pixel[j] < 0) {
                    pixel[j] = 0;
                }
                cloneImage.pixels()[i + j] = pixel[j];
            }
        }
    } else if(numComp == 1) {
        for(int i = 0; i < length; i++) {
            double addLumi = dist(engine) * m_std_dev * 255.0;
            int pixel = image.pixels()[i] + (int)addLumi;
            if(pixel > 255) {
                pixel = 255;
            } else if(pixel < 0) {
                pixel = 0;
            }
            cloneImage.pixels()[i] = pixel;
        }
    }
    return cloneImage;
}


Image ImageGenerator::hsv(const Image image, const double m_hue, const double m_saturation, const double m_value)
{
    return impl->hsv(image, m_hue, m_saturation, m_value);
}


Image ImageGeneratorImpl::hsv(const Image image, const double m_hue, const double m_saturation, const double m_value)
{
    Image cloneImage = image;
    cloneImage.setSize(image.width(), image.height(), image.numComponents());
    int width = image.width();
    int height = image.height();
    int numComp = image.numComponents();
    int length = width * height * numComp;

    for(int i = 0; i < length; i += 3) {
        int red = (int)image.pixels()[i];
        int green = (int)image.pixels()[i + 1];
        int blue = (int)image.pixels()[i + 2];

        QColor rgbColor = QColor::fromRgb(red, green, blue);
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
        cloneImage.pixels()[i] = hsvColor.red();
        cloneImage.pixels()[i + 1] = hsvColor.green();
        cloneImage.pixels()[i + 2] = hsvColor.blue();
    }
    return cloneImage;
}


Image ImageGenerator::rgb(const Image image, const double m_red, const double m_green, const double m_blue)
{
    return impl->rgb(image, m_red, m_green, m_blue);
}


Image ImageGeneratorImpl::rgb(const Image image, const double m_red, const double m_green, const double m_blue)
{
    Image cloneImage = image;
    cloneImage.setSize(image.width(), image.height(), image.numComponents());
    int width = image.width();
    int height = image.height();
    int numComp = image.numComponents();
    int length = width * height * numComp;

    for(int i = 0; i < length; i += 3) {
        int red = (int)image.pixels()[i] + m_red * 255.0;
        int green = (int)image.pixels()[i + 1] + m_green * 255.0;
        int blue = (int)image.pixels()[i + 2] + m_blue * 255.0;

        int rgb[] = { red, green, blue };
        for(int j = 0; j < 3; j++) {
            if(rgb[j] > 255) {
                rgb[j] = 255;
            } else if(rgb[j] < 0) {
                rgb[j] = 0;
            }
            cloneImage.pixels()[i + j] = rgb[j];
        }
    }
    return cloneImage;
}


Image ImageGenerator::saltPepperNoise(const Image image, const double m_salt, const double m_pepper)
{
    return impl->saltPepperNoise(image, m_salt, m_pepper);
}


Image ImageGeneratorImpl::saltPepperNoise(const Image image, const double m_salt, const double m_pepper)
{
    Image cloneImage = image;
    cloneImage.setSize(image.width(), image.height(), image.numComponents());
    int width = image.width();
    int height = image.height();
    int numComp = image.numComponents();
    int length = width * height * numComp;

    if(numComp == 3) {
        for(int i = 0; i < length; i += 3) {
            double salt = (double)(rand() % 101) / 100.0;
            if(salt < m_salt) {
                cloneImage.pixels()[i] = 255;
                cloneImage.pixels()[i + 1] = 255;
                cloneImage.pixels()[i + 2] = 255;
            }
            double pepper = (double)(rand() % 101) / 100.0;
            if(pepper < m_pepper) {
                cloneImage.pixels()[i] = 0;
                cloneImage.pixels()[i + 1] = 0;
                cloneImage.pixels()[i + 2] = 0;
            }
        }
    } else if(numComp == 1) {
        for(int i = 0; i < length; i++) {
            double salt = (double)(rand() % 101) / 100.0;
            if(salt < m_salt) {
                cloneImage.pixels()[i] = 255;
            }
            double pepper = (double)(rand() % 101) / 100.0;
            if(pepper < m_pepper) {
                cloneImage.pixels()[i] = 0;
            }
        }
    }
    return cloneImage;
}


Image ImageGenerator::filteredImage(Image image, double m_scalex, double m_scaley)
{
    return impl->filteredImage(image, m_scalex, m_scaley);
}


Image ImageGeneratorImpl::filteredImage(const Image image, const double m_scalex, const double m_scaley)
{
    //Linear interpolation
    Image cloneImage = image;
    cloneImage.setSize(image.width(), image.height(), image.numComponents());
    int width = image.width();
    int height = image.height();
    int numComp = image.numComponents();
    int length = width * height * numComp;
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

            if(numComp == 3) {
                for(int k = 0; k < 3; k++) {
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
            } else if(numComp == 1) {
                int d;
                if((m > -ys) && (m < ys) && (n >= -xs) && (n < xs)) {
                    d = (int)((1.0 - q) * ((1.0 - p) * image.pixels()[3 * ((m  + ys) * width + (n  + xs))]
                            + p * image.pixels()[3 * ((m  + ys) * width + (n + 1 + xs))])
                            + q * ((1.0 - p) * image.pixels()[3 * ((m + 1 + ys) * width + (n + xs))]
                            + p * image.pixels()[3 * ((m + 1 + ys) * width + (n + 1 + xs))]));
                } else {
                    d = 0;
                }
                if(d < 0) {
                    d = 0;
                }
                if(d > 255) {
                    d = 255;
                }
                cloneImage.pixels()[3 * ((i + ys) * width + (j + xs))] = d;
            }
        }
    }
    return cloneImage;
}
