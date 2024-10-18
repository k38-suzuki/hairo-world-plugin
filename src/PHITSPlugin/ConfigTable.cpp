/**
   @author Kenta Suzuki
*/

#include "ConfigTable.h"
#include <cnoid/YAMLReader>
#include "gettext.h"

using namespace std;
using namespace cnoid;


NuclideTable::NuclideTable()
{
    nuclideName_.clear();
    sourceEnergy_.clear();
    sourceIncidenceRate_.clear();
}


bool NuclideTable::load(const string& filename, ostream& os)
{
    nuclideName_.clear();
    sourceEnergy_.clear();
    sourceIncidenceRate_.clear();

    try {
        YAMLReader reader;
        MappingPtr node = reader.loadDocument(filename)->toMapping();
        if(node) {
            auto& nuclideList = *node->findListing("nuclides");
            if(nuclideList.isValid()) {
                sourceEnergy_.resize(nuclideList.size());
                sourceIncidenceRate_.resize(nuclideList.size());
                for(int i = 0; i < nuclideList.size(); ++i) {
                    Mapping* info = nuclideList[i].toMapping();
                    string name;
                    info->read("name", name);
                    auto& energyList = *info->findListing("energy");
                    auto& photonList = *info->findListing("photonIncidenceRate");
                    sourceEnergy_[i].resize(energyList.size());
                    sourceIncidenceRate_[i].resize(energyList.size());

                    for(int j = 0; j < energyList.size(); ++j) {
                        sourceEnergy_[i][j] = energyList[j].toDouble();
                    }

                    if(energyList.size() != photonList.size()) {
                        ValueNode::SyntaxException ex;
                        ex.setPosition(ex.line(), ex.column());
                        ex.setMessage(_("The list is incomplete"));
                        throw ex;
                    }

                    for(int j = 0; j < photonList.size(); ++j) {
                        sourceIncidenceRate_[i][j] = photonList[j].toDouble();
                    }
                    nuclideName_.push_back(make_tuple(name, energyList.size()));
                }
            }
        }
    }
    catch (const ValueNode::Exception& ex) {
        os << ex.message();
    }
    return true;
}


ElementTable::ElementTable()
{
    matData_.clear();
    element_.clear();
    elementId_.clear();
    weightRate_.clear();
    atomicNum_.clear();
}


int ElementTable::materialId(string& material) const
{
    int materialId = 1;
    int id = 1;
    if(!material.empty()) {
        for(auto& item : matData_) {
            string name = get<0>(item);
            if(material == name) {
               materialId = max(1, min(99, id));
            }
            id++;
        }
    }
    return materialId;
}


bool ElementTable::load(const string& filename, ostream& os)
{
    matData_.clear();
    element_.clear();
    elementId_.clear();
    weightRate_.clear();
    atomicNum_.clear();

    try {
        YAMLReader reader;
        MappingPtr node = reader.loadDocument(filename)->toMapping();
        if(node) {
            auto& elementList = *node->findListing("elements");
            if(elementList.isValid()) {
                element_.resize(elementList.size());
                elementId_.resize(elementList.size());
                weightRate_.resize(elementList.size());
                for(int i = 0; i < elementList.size(); ++i) {
                    Mapping* info = elementList[i].toMapping();
                    string name;
                    info->read("name", name);
                    double density;
                    info->read("density", density);
                    auto& compoList = *info->findListing("elementComposition");
                    auto& atomicList = *info->findListing("atomicNumber");
                    auto& rateList = *info->findListing("elementWeightRate");

                    int numElement = compoList.size();
                    element_[i].resize(numElement);
                    elementId_[i].resize(numElement);
                    weightRate_[i].resize(numElement);

                    for(int j = 0; j < compoList.size(); ++j) {
                        element_[i][j] = compoList[j].toString();
                    }

                    for(int j = 0; j < atomicList.size(); ++j) {
                        atomicNum_.push_back(atomicList[j].toInt());
                        elementId_[i][j] = atomicList[j].toInt();
                    }

                    if(compoList.size() != rateList.size()) {
                        ValueNode::SyntaxException ex;
                        ex.setPosition(ex.line(), ex.column());
                        ex.setMessage(_("The list is incomplete"));
                        throw ex;
                    }

                    if(atomicList.size() != rateList.size()) {
                        ValueNode::SyntaxException ex;
                        ex.setPosition(ex.line(), ex.column());
                        ex.setMessage(_("The list is incomplete"));
                        throw ex;
                    }

                    for(int j = 0; j < rateList.size(); j++) {
                        weightRate_[i][j] = rateList[j].toDouble();
                    }

                    matData_.push_back(make_tuple(name, numElement, density));
                }
            }
        }
    }
    catch (const ValueNode::Exception& ex) {
        os << ex.message();
    }
    return true;
}
