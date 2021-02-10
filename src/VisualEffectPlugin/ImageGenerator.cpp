/**
   \file
   \author Kenta Suzuki
*/

#include "ImageGenerator.h"
#include <cmath>
#include <algorithm>
#include <random>

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
    Image filteredImage(Image image, double m_scalex, double m_scaley);
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
    int width = cloneImage.width();
    int height = cloneImage.height();
    int numComp = cloneImage.numComponents();
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
                }
                else if(numComp == 1) {
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
    int width = cloneImage.width();
    int height = cloneImage.height();
    int numComp = cloneImage.numComponents();
    int length = width * height * numComp;

    if(numComp == 3) {
        for(int i = 0; i < length; i += 3) {
            double addLumi = dist(engine) * m_std_dev * 255.0;
            int pixel[3];

            for(int j = 0; j < 3; j++) {
                pixel[j] = cloneImage.pixels()[i + j] + (int)addLumi;
                if(pixel[j] < 0) {
                    cloneImage.pixels()[i + j] = 0;
                } else if(pixel[j] > 255) {
                    cloneImage.pixels()[i + j] = 255;
                } else {
                    cloneImage.pixels()[i + j] = pixel[j];
                }
            }
        }
    }
    else if(numComp == 1) {
        for(int i = 0; i < length; i++) {
            double addLumi = dist(engine) * m_std_dev * 255.0;
            int pixel = cloneImage.pixels()[i] + (int)addLumi;
            if(pixel < 0) {
                cloneImage.pixels()[i] = 0;
            } else if(pixel > 255) {
                cloneImage.pixels()[i] = 255;
            } else {
                cloneImage.pixels()[i] = pixel;
            }
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
    int width = cloneImage.width();
    int height = cloneImage.height();
    int numComp = cloneImage.numComponents();
    int length = width * height * numComp;

    for(int i = 0; i < length; i += 3) {
        //rgb to hsv
        float red = (float)cloneImage.pixels()[i] / 255.0;
        float green = (float)cloneImage.pixels()[i + 1] / 255.0;
        float blue = (float)cloneImage.pixels()[i + 2] / 255.0;
        float max = std::max({ red, green, blue });
        float min = std::min({ red, green, blue });
        float diff = max - min;
        float hue;
        float saturation = diff / max;
        float value = max;

        if(max == red) {
            hue = 60.0 * (green - blue) / diff;
        }
        else if(max == green) {
            hue = 60.0 * (blue - red) / diff + 120.0;
        }
        else if(max == blue) {
            hue = 60.0 * (red - green) / diff + 240.0;
        }

        if(hue < 0.0) {
            hue += 360.0;
        }

        //hsv to rgb
        float h = hue + m_hue * 360.0;
        float s = saturation + m_saturation;
        float v = value + m_value;
        float rgb[3];

        if(h >= 360.0) {
            while(h >= 360.0) {
                h -= 360.0;
            }
        }
        else if(h < 0.0) {
            while (h < 0.0) {
                h += 360.0;
            }
        }

        if(s > 1.0) {
            s = 1.0;
        }
        else if(s < 0.0) {
            s = 0.0;
        }

        if(v > 1.0) {
            v = 1.0;
        }
        else if(v < 0.0) {
            v = 0.0;
        }

        if(s > 0.0) {
            float dh = std::floor(h / 60.0);
            float p = v * (1.0 - s);
            float q = v * (1.0 - s * (h / 60.0 - dh));
            float t = v * (1.0 - s * (1.0 - (h / 60.0 - dh)));

            switch ((int)dh) {
            case 0:
                rgb[0] = v;
                rgb[1] = t;
                rgb[2] = p;
                break;
            case 1:
                rgb[0] = q;
                rgb[1] = v;
                rgb[2] = p;
                break;
            case 2:
                rgb[0] = p;
                rgb[1] = v;
                rgb[2] = t;
                break;
            case 3:
                rgb[0] = p;
                rgb[1] = q;
                rgb[2] = v;
                break;
            case 4:
                rgb[0] = t;
                rgb[1] = p;
                rgb[2] = v;
                break;
            case 5:
                rgb[0] = v;
                rgb[1] = p;
                rgb[2] = q;
                break;
            default:
                break;
            }
        } else {
            rgb[0] = v;
            rgb[1] = v;
            rgb[2] = v;
        }

        for(int j = 0; j < 3; j++) {
            rgb[j] *= 255.0;
            if(rgb[j] > 255.0) {
                rgb[j] = 255.0;
            }
            else if(rgb[j] < 0.0) {
                    rgb[j] = 0.0;
            }
            cloneImage.pixels()[i + j] = (int)(rgb[j]);
        }
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
    int width = cloneImage.width();
    int height = cloneImage.height();
    int numComp = cloneImage.numComponents();
    int length = width * height * numComp;

    int coefLumi[] = { (int)(255.0 * m_red), (int)(255.0 * m_green), (int)(255.0 * m_blue) };

    for(int i = 0; i < length; i += 3) {
        for(int j = 0; j < 3; j++) {
            int lumi = cloneImage.pixels()[i + j] + coefLumi[j];
            if(lumi > 255) {
                lumi = 255;
            }
            else if(lumi < 0) {
                lumi = 0;
            }
            cloneImage.pixels()[i + j] = lumi;
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
    int width = cloneImage.width();
    int height = cloneImage.height();
    int numComp = cloneImage.numComponents();
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
    }
    else if(numComp == 1) {
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


Image filteredImage(Image image, double m_scalex, double m_scaley)
{
    //Linear interpolation
    Image cloneImage = image;
    cloneImage.setSize(image.width(), image.height(), image.numComponents());
    int width = cloneImage.width();
    int height = cloneImage.height();
    int numComp = cloneImage.numComponents();
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
