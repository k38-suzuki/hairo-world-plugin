/**
   @author Kenta Suzuki
*/

#include "VisualFilter.h"

using namespace cnoid;

namespace {

unsigned long xorshift128()
{
    static unsigned long x = 123456789;
    static unsigned long y = 362436069;
    static unsigned long z = 521288629;
    static unsigned long w = 88675123;
    unsigned long t = (x ^ (x << 11));
    x = y;
    y = z;
    z = w;
    w = (w ^ (w >> 19)) ^ (t ^ (t >> 8));
    return w;
}

inline unsigned long rand(int min, int max) { return min + xorshift128() % (max - min); }

}


VisualFilter::VisualFilter()
{
    width_ = 0;
    height_ = 0;
    std::default_random_engine engine_init(seed_gen_());
    engine_ = engine_init;
    std::normal_distribution<> dist_init(0.0, 1.0);
    dist_ = dist_init;
}


void VisualFilter::initialize(int width, int height)
{
    width_ = width;
    height_ = height;
}


void VisualFilter::red(Image* image)
{
    image->setSize(width_, height_, 3);
    unsigned char* pixels = image->pixels();

    for(int j = 0; j < height_; ++j) {
        for(int i = 0; i < width_; ++i) {
            unsigned char* pix = &pixels[(i + j * width_) * 3];
            pix[0] = 255;
            pix[1] = pix[2] = 0;
        }
    }
}


void VisualFilter::green(Image* image)
{
    image->setSize(width_, height_, 3);
    unsigned char* pixels = image->pixels();

    for(int j = 0; j < height_; ++j) {
        for(int i = 0; i < width_; ++i) {
            unsigned char* pix = &pixels[(i + j * width_) * 3];
            pix[1] = 255;
            pix[0] = pix[2] = 0;
        }
    }
}


void VisualFilter::blue(Image* image)
{
    image->setSize(width_, height_, 3);
    unsigned char* pixels = image->pixels();

    for(int j = 0; j < height_; ++j) {
        for(int i = 0; i < width_; ++i) {
            unsigned char* pix = &pixels[(i + j * width_) * 3];
            pix[2] = 255;
            pix[0] = pix[1] = 0;
        }
    }
}


void VisualFilter::white(Image* image)
{
    image->setSize(width_, height_, 3);
    unsigned char* pixels = image->pixels();

    for(int j = 0; j < height_; ++j) {
        for(int i = 0; i < width_; ++i) {
            unsigned char* pix = &pixels[(i + j * width_) * 3];
            pix[0] = pix[1] = pix[2] = 255;
        }
    }
}


void VisualFilter::black(Image* image)
{
    image->setSize(width_, height_, 3);
    unsigned char* pixels = image->pixels();

    for(int j = 0; j < height_; ++j) {
        for(int i = 0; i < width_; ++i) {
            unsigned char* pix = &pixels[(i + j * width_) * 3];
            pix[0] = pix[1] = pix[2] = 0;
        }
    }
}


void VisualFilter::salt(Image* image, const double& salt_amount)
{
    image->setSize(width_, height_, 3);
    unsigned char* pixels = image->pixels();

    for(int j = 0; j < height_; ++j) {
        for(int i = 0; i < width_; ++i) {
            unsigned char* pix = &pixels[(i + j * width_) * 3];
            double r = (double)(rand(0, 100)) / 100.0;
            if(r < salt_amount) {
                pix[0] = pix[1] = pix[2] = 255;
            }
        }
    }
}


void VisualFilter::random_salt(Image* image, const double& salt_amount, const double& salt_chance)
{
    double r = (double)(rand(0, 100)) / 100.0;
    if(r < salt_chance) {
        this->salt(image, salt_amount);
    }
}


void VisualFilter::pepper(Image* image, const double& pepper_amount)
{
    image->setSize(width_, height_, 3);
    unsigned char* pixels = image->pixels();

    for(int j = 0; j < height_; ++j) {
        for(int i = 0; i < width_; ++i) {
            unsigned char* pix = &pixels[(i + j * width_) * 3];
            double r = (double)(rand(0, 100)) / 100.0;
            if(r < pepper_amount) {
                pix[0] = pix[1] = pix[2] = 0;
            }
        }
    }
}


void VisualFilter::random_pepper(Image* image, const double& pepper_amount, const double& pepper_chance)
{
    double r = (double)(rand(0, 100)) / 100.0;
    if(r < pepper_chance) {
        this->pepper(image, pepper_amount);
    }
}


void VisualFilter::salt_pepper(Image* image, const double& salt_amount, const double& pepper_amount)
{
    image->setSize(width_, height_, 3);
    unsigned char* pixels = image->pixels();

    for(int j = 0; j < height_; ++j) {
        for(int i = 0; i < width_; ++i) {
            unsigned char* pix = &pixels[(i + j * width_) * 3];
            double salt = (double)(rand(0, 100)) / 100.0;
            double pepper = (double)(rand(0, 100)) / 100.0;
            if(salt < salt_amount) {
                pix[0] = pix[1] = pix[2] = 255;
            }
            if(pepper < pepper_amount) {
                pix[0] = pix[1] = pix[2] = 0;
            }
        }
    }
}


void VisualFilter::rgb(Image* image, const double& red, const double& green, const double& blue)
{
    image->setSize(width_, height_, 3);
    unsigned char* pixels = image->pixels();

    for(int j = 0; j < height_; ++j) {
        for(int i = 0; i < width_; ++i) {
            unsigned char* pix = &pixels[(i + j * width_) * 3];
            pix[0] += 255 * red;
            pix[1] += 255 * green;
            pix[2] += 255 * blue;
        }
    }
}


void VisualFilter::hsv(Image* image, const double& hue, const double& saturation, const double& value)
{
    image->setSize(width_, height_, 3);
    unsigned char* pixels = image->pixels();

    for(int j = 0; j < height_; ++j) {
        for(int i = 0; i < width_; ++i) {
            unsigned char* pix = &pixels[(i + j * width_) * 3];
            QColor rgb = QColor::fromRgb(pix[0], pix[1], pix[2]);
            int h = rgb.hue() + hue * 360.0;
            int s = rgb.saturation() + saturation * 255.0;
            int v = rgb.value() + value * 255.0;

            h = h > 359 ? h - 360 : h;
            h = h < 0 ? 0 : h;
            s = s > 255 ? 255 : s;
            s = s < 0 ? 0 : s;
            v = v > 255 ? 255 : v;
            v = v < 0 ? 0 : v;

            QColor hsv = QColor::fromHsv(h, s, v);
            pix[0] = hsv.red();
            pix[1] = hsv.green();
            pix[2] = hsv.blue();
        }
    }
}


void VisualFilter::gaussian_noise(Image* image, const double& std_dev)
{
    image->setSize(width_, height_, 3);
    unsigned char* pixels = image->pixels();

    for(int j = 0; j < height_; ++j) {
        for(int i = 0; i < width_; ++i) {
            unsigned char* pix = &pixels[(i + j * width_) * 3];
            double c = 255 * std_dev * dist_(engine_);
            pix[0] += c;
            pix[1] += c;
            pix[2] += c;
        }
    }
}


void VisualFilter::barrel_distortion(Image* image, const double& coef_b, const double& coef_d)
{
    image->setSize(width_, height_, 3);
    unsigned char* pixels = image->pixels();

    std::shared_ptr<Image> srcImage = std::make_shared<Image>(*image);
    unsigned char* src = srcImage->pixels();
    this->black(image);

    double coefa = 0.0;
    double coefb = coef_b;
    double coefc = 0.0;
    double coefd = coef_d - coefa - coefb - coefc;

    for(int j = 0; j < height_; ++j) {
        for(int i = 0; i < width_; ++i) {
            unsigned char* pix = &pixels[(i + j * width_) * 3];
            int d = std::min(width_, height_) / 2;
            double cntx = (width_ - 1) / 2.0;
            double cnty = (height_ - 1) / 2.0;
            double delx = (i - cntx) / d;
            double dely = (j - cnty) / d;
            double dstr = sqrt(delx * delx + dely * dely);
            double srcr = (coefa * dstr * dstr * dstr + coefb * dstr * dstr + coefc * dstr + coefd) * dstr;
            double fctr = abs(dstr / srcr);
            double srcxd = cntx + (delx * fctr * d);
            double srcyd = cnty + (dely * fctr * d);
            int srcx = (int)srcxd;
            int srcy = (int)srcyd;
            unsigned char* pix2 = &src[(srcy * width_ + srcx) * 3];
            if((srcx >= 0) && (srcy >= 0) && (srcx < width_) && (srcy < height_)) {
                pix[0] = pix2[0];
                pix[1] = pix2[1];
                pix[2] = pix2[2];
            }
        }
    }
}


void VisualFilter::mosaic(Image* image, int kernel)
{
    image->setSize(width_, height_, 3);
    unsigned char* pixels = image->pixels();

    std::shared_ptr<Image> srcImage = std::make_shared<Image>(*image);
    unsigned char* src = srcImage->pixels();

    int r, g, b;
    int margin_x, margin_y;

    for(int j = 0; j < height_; j += kernel) {
        for(int i = 0; i < width_; i += kernel) {
            r = g = b = 0;
            for(int y = 0; y < kernel; ++y) {
                if(height_ <= j + y) {
                    break;
                }
                margin_y = y + 1;
                for(int x = 0; x < kernel; ++x) {
                    if(width_ <= i + x) {
                        break;
                    }
                    margin_x = x + 1;
                    unsigned char* pix2 = &src[((i + x) + (j + y) * width_) * 3];
                    r += pix2[0];
                    g += pix2[1];
                    b += pix2[2];
                }
            }

            for(int y = 0; y < margin_y; ++y) {
                for(int x = 0; x < margin_x; ++x) {
                    unsigned char* pix = &pixels[((i + x) + (j + y) * width_) * 3];
                    pix[0] = r / (margin_x * margin_y);
                    pix[1] = g / (margin_x * margin_y);
                    pix[2] = b / (margin_x * margin_y);
                }
            }
        }
    }
}


void VisualFilter::random_mosaic(Image* image, const double& rate, int kernel)
{
    double r = (double)(rand(0, 100)) / 100.0;
    if(r < rate) {
        mosaic(image, kernel);
    }
}


namespace cnoid {

void toCnoidImage(Image* image, QImage q_image)
{
    int width = q_image.width();
    int height = q_image.height();

    image->setSize(width, height, 3);
    unsigned char* pixels = image->pixels();

    for(int j = 0; j < height; ++j) {
        for(int i = 0; i < width; ++i) {
            unsigned char* pix = &pixels[(i + j * width) * 3];
            QRgb c = q_image.pixel(i, j);
            pix[0] = qRed(c);
            pix[1] = qGreen(c);
            pix[2] = qBlue(c);
        }
    }
}


QImage toQImage(Image* image)
{
    int width = image->width();
    int height = image->height();
    unsigned char* pixels = image->pixels();

    QImage q_image(width, height, QImage::Format_RGB888);
    for(int j = 0; j < height; ++j) {
        for(int i = 0; i < width; ++i) {
            unsigned char* pix = &pixels[(i + j * width) * 3];
            QColor c(pix[0], pix[1], pix[2]);
            q_image.setPixelColor(i, j, c);
        }
    }
    return q_image;
}

}
