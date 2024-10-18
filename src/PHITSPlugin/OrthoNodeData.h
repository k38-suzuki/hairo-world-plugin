/**
   @author Kenta Suzuki
*/

#ifndef CNOID_PHITS_PLUGIN_ORTHO_GRID_NODE_DATA_H
#define CNOID_PHITS_PLUGIN_ORTHO_GRID_NODE_DATA_H

#include <cnoid/Referenced>
#include <cstdint>
#include <memory>
#include <vector>
#include "Array3D.h"
#include "Box.h"
#include "GammaData.h"

namespace cnoid {

class OrthoNodeData : public Referenced
{
public:
    OrthoNodeData(const GammaData& gammaData);
    virtual ~OrthoNodeData();

    enum AxisID {
        X_AXIS,
        Y_AXIS,
        Z_AXIS,
        NumAxes
    };

    bool createShieldData(std::string& filename, const GammaData& gammaData);
    static void addShield(const std::string& material, const double& thickness);
    static void clearShield();

    bool isValid() const { return isValid_; }
    void clear();
    size_t size(const int axis) const { return coordinates_[axis].size() - 1; }
    std::vector<double> coordinates(const int axis) const { return coordinates_[axis]; }
    double min() const { return min_; }
    double max() const { return max_; }
    double value(const uint32_t x, const uint32_t y, const uint32_t z) const { return cell_(x, y, z); }
    double value_shield(int id, const uint32_t x, const uint32_t y, const uint32_t z) const { return cell_shield_[id](x, y, z); }
    double value_node(const uint32_t x, const uint32_t y, const uint32_t z) const { return node_(x, y, z); }
    Boxd bounds() const {
        Vector3d min(coordinates_[X_AXIS].front() ,coordinates_[Y_AXIS].front(), coordinates_[Z_AXIS].front());
        Vector3d max(coordinates_[X_AXIS].back() ,coordinates_[Y_AXIS].back(), coordinates_[Z_AXIS].back());
        return Boxd(min, max);
    }

    Boxd cellBounds(const uint32_t i, const uint32_t j, const uint32_t k) const {
        Vector3d minPos(coordinates_[X_AXIS][i], coordinates_[Y_AXIS][j], coordinates_[Z_AXIS][k]);
        Vector3d maxPos(coordinates_[X_AXIS][i + 1], coordinates_[Y_AXIS][j + 1], coordinates_[Z_AXIS][k + 1]);
        return Boxd(minPos, maxPos);
    }

    double value(const Vector3d& pos) const;
    bool findCellIndex(const Vector3d& pos, uint32_t& i, uint32_t& j, uint32_t& k) const;

private:
    bool isValid_;
    double min_;
    double max_;
    array3d cell_;
    array3d* cell_shield_;
    array3d node_;
    std::vector<double> coordinates_[NumAxes];
};

typedef ref_ptr<OrthoNodeData> OrthoNodeDataPtr;

}

#endif // CNOID_PHITS_PLUGIN_ORTHO_GRID_NODE_DATA_H
