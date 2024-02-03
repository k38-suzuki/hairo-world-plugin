/**
   @author Kenta Suzuki
*/

#ifndef CNOID_PHITS_PLUGIN_GAMMA_IMAGE_GENERATOR_H
#define CNOID_PHITS_PLUGIN_GAMMA_IMAGE_GENERATOR_H

#include <cnoid/Camera>
#include <cnoid/Image>

namespace cnoid {

class GammaImageGenerator
{
public:
    GammaImageGenerator();
    virtual ~GammaImageGenerator();

    void generateImage(Camera* camera, std::shared_ptr<Image>& image);

private:
    class Impl;
    Impl* impl;
};

}

#endif
