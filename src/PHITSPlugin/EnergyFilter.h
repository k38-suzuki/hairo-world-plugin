/**
   \file
   \author Kenta Suzuki
*/

#ifndef CNOID_PHITSPLUGIN_ENERGYFILTER_H
#define CNOID_PHITSPLUGIN_ENERGYFILTER_H

#include <cnoid/Archive>
#include <cnoid/NullOut>
#include <vector>

namespace cnoid {

class EnergyFilterImpl;

class EnergyFilter
{
public:
    EnergyFilter();
    virtual ~EnergyFilter();

    enum ModeID {
        NO_FILTER,
        RANGE_FILTER,
        NUCLIDE_FILTER,
        NUM_FILTERS
    };

    void showConfigDialog();
    int mode() const;
    int min() const;
    int max() const;

    struct NuclideFilterInfo {
        std::string species;
        int min;
        int max;
        bool check;
    };

    std::vector<EnergyFilter::NuclideFilterInfo> nuclideFilterInfo() const;

    bool load(const std::string& filename, std::ostream& os = nullout());

    bool store(Archive& archive);
    bool restore(const Archive& archive);

private:
    EnergyFilterImpl* impl;
    friend class EnergyFilterImpl;
};

}

#endif // CNOID_PHITSPLUGIN_ENERGYFILTER_H
