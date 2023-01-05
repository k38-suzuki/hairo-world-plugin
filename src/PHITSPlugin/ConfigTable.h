/**
   \file
   \author Kenta Suzuki
*/

#ifndef CNOID_PHITS_PLUGIN_CONFIG_TABLE_H
#define CNOID_PHITS_PLUGIN_CONFIG_TABLE_H

#include <cnoid/NullOut>
#include <vector>

namespace cnoid {

class NuclideTable
{
public:
    NuclideTable();
    virtual ~NuclideTable() { }

    bool load(const std::string& filename, std::ostream& os = nullout());

    const std::vector<std::tuple<std::string, int>>& nuclideName() const { return nuclideName_; }
    const std::vector<std::vector<double>>& sourceEnergy() const { return sourceEnergy_; }
    const std::vector<std::vector<double>>& sourceIncidenceRate() const { return sourceIncidenceRate_; }

    //double sourceEnergy(int id, int iEne) { return sourceEnergy[id][iEne]; }
    //double sourceIncidenceRate(int id, int iEne) { return sourceIncidenceRate[id][iEne]; }

private:
    std::vector<std::tuple<std::string, int>> nuclideName_;
    std::vector<std::vector<double>> sourceEnergy_;
    std::vector<std::vector<double>> sourceIncidenceRate_;
};


class ElementTable
{
public:
    ElementTable();
    virtual ~ElementTable() { }

    bool load(const std::string& filename, std::ostream& os = nullout());

    int materialId(std::string& material) const;

    const std::vector<std::tuple<std::string, int, double>>& matData() const { return matData_; }
    const std::vector<std::vector<std::string>>& element() const { return element_; }
    const std::vector<std::vector<int>>& elementId() const { return elementId_; }
    const std::vector<std::vector<double>>& weightRate() const { return weightRate_; }
    std::vector<int>& atomicNum() { return atomicNum_; }

    //std::string element(int im, int id) { return element[im][id]; }
    //double weightRate(int im, int id)  { return weightRate[im][id]; }
    //int elementId(int im, int id) { return elementId[im][id]; }

private:
    std::vector<std::tuple<std::string, int, double>> matData_;
    std::vector<std::vector<std::string>> element_;
    std::vector<std::vector<int>> elementId_;
    std::vector<std::vector<double>> weightRate_;
    std::vector<int> atomicNum_;
};

}

#endif // CNOID_PHITS_PLUGIN_CONFIG_TABLE_H