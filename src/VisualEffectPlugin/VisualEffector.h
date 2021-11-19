/**
   \file
   \author Kenta Suzuki
*/

#ifndef CNOID_VISUALEFFECTPLUGIN_VISUALEFFECTOR_H
#define CNOID_VISUALEFFECTPLUGIN_VISUALEFFECTOR_H

#include <cnoid/Archive>

namespace cnoid {

class VisualEffectorImpl;

class VisualEffector
{
public:
    VisualEffector();
    virtual ~VisualEffector();

    void show();

    double hue() const;
    double saturation() const;
    double value() const;
    double red() const;
    double green() const;
    double blue() const;
    double coefB() const;
    double coefD() const;
    double stdDev() const;
    double salt() const;
    double pepper() const;
    bool flip() const;
    int filter() const;

    bool store(Archive& archive);
    bool restore(const Archive& archive);

private:
    VisualEffectorImpl* impl;
    friend class VisualEffectorImpl;
};

}

#endif // CNOID_VISUALEFFECTPLUGIN_VISUALEFFECTOR_H
