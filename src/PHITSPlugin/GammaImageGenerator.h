/**
   \file
   \author Kenta Suzuki
*/

#ifndef CNOID_PHITSPLUGIN_GAMMAIMAGEGENERATOR_H
#define CNOID_PHITSPLUGIN_GAMMAIMAGEGENERATOR_H

#include <cnoid/Camera>
#include <cnoid/Image>

namespace cnoid {

class GammaImageGeneratorImpl;

class GammaImageGenerator
{
public:
    GammaImageGenerator();
    virtual ~GammaImageGenerator();

    void generateImage(Camera* camera, std::shared_ptr<Image>& image);

private:
    GammaImageGeneratorImpl* impl;
    friend class GammaImageGeneratorImpl;
};

}

#endif // CNOID_PHITSPLUGIN_GAMMAIMAGEGENERATOR_H
