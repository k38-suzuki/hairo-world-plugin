/**
   @author Kenta Suzuki
*/
#ifndef CNOID_PHITS_PLUGIN_ARRAY_3D_H
#define CNOID_PHITS_PLUGIN_ARRAY_3D_H

#include <vector>

namespace cnoid {

template<class T> class Array3D
{
public:
    Array3D()
    {
        size_x_ = size_y_ = size_z_ = 0;
        array_.clear();
    }

    virtual ~Array3D() { }

    bool resize(unsigned int x = 0, unsigned int y = 1, unsigned int z = 1)
    {
        if(x < 0 || y < 0 || z < 0) {
            return false;
        }
        array_.resize(x * y * z);
        size_x_ = x;
        size_y_ = y;
        size_z_ = z;
        return true;
    }

    T* front() const { return array_.empty() ? nullptr : &(array_[0]); }

    T operator()(unsigned int x, unsigned int y = 0, unsigned int z = 0) const { return array_[size_y_ * size_x_ * z + size_x_ * y + x]; }
    T& operator()(unsigned int x, unsigned int y = 0, unsigned int z = 0) { return array_[size_y_ * size_x_ * z + size_x_ * y + x]; }

    unsigned int size_x() const { return size_x_; }
    unsigned int size_y() const { return size_y_; }
    unsigned int size_z() const { return size_z_; }

private:
    unsigned int size_x_;
    unsigned int size_y_;
    unsigned int size_z_;
    std::vector<T> array_;
};

typedef Array3D<int> array3i;
typedef Array3D<float> array3f;
typedef Array3D<double> array3d;

}

#endif
