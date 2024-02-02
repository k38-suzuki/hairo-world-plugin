/**
   @author Kenta Suzuki
*/

#include "DoseSimulatorItem.h"
#include <cnoid/Archive>
#include <cnoid/ExecutablePath>
#include <cnoid/ItemManager>
#include <cnoid/MessageView>
#include <cnoid/PutPropertyFunction>
#include <cnoid/Selection>
#include <cnoid/SimulatorItem>
#include <cnoid/UTF8>
#include <cnoid/WorldItem>
#include <fmt/format.h>
#include <map>
#include "ColorScale.h"
#include "CrossSectionItem.h"
#include "DoseMeter.h"
#include "GammaData.h"
#include "gettext.h"

#ifndef _MSC_VER
#include <alloca.h>
#endif

using namespace cnoid;
using namespace std;

namespace cnoid {

class DoseSimulatorItemImpl
{
public:
    DoseSimulatorItemImpl(DoseSimulatorItem* self);
    DoseSimulatorItemImpl(DoseSimulatorItem* self, const DoseSimulatorItemImpl& org);
    DoseSimulatorItem* self;

    DeviceList<DoseMeter> doseMeters;
    double worldTimeStep;
    double timeUnit;
    OrthoNodeDataPtr nodeData;
    string defaultShieldTableFile;
    CrossSectionItem* crossSectionItem;
    bool isLoaded;

    Selection colorScale;

    enum ColorScaleID { LOG_SCALE, LINER_SCALE, NUM_SCALES };

    bool initializeSimulation(SimulatorItem* simulatorItem);
    void onMidDynamicsFunction();
    void onPostDynamicsFunction();
    void setDefaultShieldFile(const string& filename);
    bool onColorScalePropertyChanged(const int& index);
    void onGammaDataLoaded();
    void doPutProperties(PutPropertyFunction& putProperty);
    bool store(Archive& archive);
    bool restore(const Archive& archive);
};

}


void DoseSimulatorItem::initializeClass(ExtensionManager* ext)
{
    ext->itemManager().registerClass<DoseSimulatorItem, SubSimulatorItem>(N_("DoseSimulatorItem"));
    ext->itemManager().addCreationPanel<DoseSimulatorItem>();
}


DoseSimulatorItem::DoseSimulatorItem()
{
    impl = new DoseSimulatorItemImpl(this);
}


DoseSimulatorItemImpl::DoseSimulatorItemImpl(DoseSimulatorItem* self)
    : self(self)
{
    doseMeters.clear();
    worldTimeStep = 0.0;
    timeUnit = 3600.0;
    nodeData = nullptr;
    defaultShieldTableFile = toUTF8((shareDirPath() / "default" / "shields.yaml").string());
    crossSectionItem = nullptr;
    isLoaded = false;
    colorScale.setSymbol(LOG_SCALE, N_("Log"));
    colorScale.setSymbol(LINER_SCALE, N_("Liner"));
}


DoseSimulatorItem::DoseSimulatorItem(const DoseSimulatorItem& org)
    : SubSimulatorItem(org)
{
    impl = new DoseSimulatorItemImpl(this, *org.impl);
}


DoseSimulatorItemImpl::DoseSimulatorItemImpl(DoseSimulatorItem* self, const DoseSimulatorItemImpl& org)
    : self(self),
      defaultShieldTableFile(org.defaultShieldTableFile)
{
    isLoaded = org.isLoaded;
    colorScale = org.colorScale;
}


DoseSimulatorItem::~DoseSimulatorItem()
{
    delete impl;
}


bool DoseSimulatorItem::initializeSimulation(SimulatorItem* simulatorItem)
{
    return impl->initializeSimulation(simulatorItem);
}


bool DoseSimulatorItemImpl::initializeSimulation(SimulatorItem* simulatorItem)
{
    worldTimeStep = simulatorItem->worldTimeStep();
    doseMeters.clear();
    crossSectionItem = nullptr;

    const vector<SimulationBody*>& simBodies = simulatorItem->simulationBodies();
    for(size_t i = 0; i < simBodies.size(); ++i) {
        Body* body = simBodies[i]->body();
        doseMeters << body->devices();
    }

    OrthoNodeData::clearShield();
    for(size_t i = 0; i < doseMeters.size(); ++i) {
        DoseMeter* doseMeter = doseMeters[i];
        OrthoNodeData::addShield(doseMeter->material(), doseMeter->thickness());
    }

    WorldItem* worldItem = simulatorItem->findOwnerItem<WorldItem>();
    if(worldItem) {
        ItemList<CrossSectionItem> items = worldItem->descendantItems<CrossSectionItem>();
        if(items.size()) {
            crossSectionItem = items[0];
        }
    }

    if(crossSectionItem) {
        onGammaDataLoaded();
        if(nodeData) {
            simulatorItem->addMidDynamicsFunction([&](){ onMidDynamicsFunction(); });
            simulatorItem->addPostDynamicsFunction([&](){ onPostDynamicsFunction(); });
            crossSectionItem->sigGammaDataLoaded().connect([&](){ onGammaDataLoaded(); });
        }
    } else {
        MessageView::instance()->putln(fmt::format(_("GammaData was not found.")));
        return false;
    }

    return true;
}


void DoseSimulatorItemImpl::onMidDynamicsFunction()
{
    if(!nodeData) {
        return;
    }
    for(size_t i = 0; i < doseMeters.size(); ++i) {
        DoseMeter* doseMeter = doseMeters[i];
        Link* link = doseMeter->link();
        link->body()->calcCenterOfMass();

        Vector3 position = link->centerOfMassGlobal();
        Boxd gridBox = nodeData->bounds();
        double integralDose = doseMeter->integralDose();
        double doseRate = 0.0;
        if(gridBox.contain(link->centerOfMassGlobal())) {
           uint32_t x = 0;
           uint32_t y = 0;
           uint32_t z = 0;
           if(nodeData->findCellIndex(position, x, y, z)) {
               if(!doseMeter->isShield()) {
                   doseRate = nodeData->value(x, y, z);
                   integralDose += doseRate * worldTimeStep / timeUnit;
               } else {
                   if(isLoaded) {
                       doseRate = nodeData->value_shield(i, x, y, z);
                   }
                   integralDose += doseRate * worldTimeStep / timeUnit;
               }
               doseMeter->setDoseRate(doseRate);
               doseMeter->setIntegralDose(integralDose);
               doseMeter->notifyStateChange();
           }
        }
    }
}


void DoseSimulatorItemImpl::onPostDynamicsFunction()
{
    if(doseMeters.size()) {
        vector<double> integralDoses;
        for(size_t i = 0; i < doseMeters.size(); ++i) {
            DoseMeter* doseMeter = doseMeters[i];
            integralDoses.push_back(doseMeter->integralDose());
        }

        double min = *min_element(integralDoses.begin(), integralDoses.end());
        double max = *max_element(integralDoses.begin(), integralDoses.end());
        ColorScale* scale = new ColorScale();
        int exp = (int)floor(log10(fabs(max))) + 1;
        min = 1.0 * pow(10, exp - 6);
        max = 1.0 * pow(10, exp);
        scale->setRange(min, max);

        for(size_t i = 0; i < doseMeters.size(); ++i) {
            DoseMeter* doseMeter = doseMeters[i];
            Vector3 color;
            if(colorScale.is(LOG_SCALE)) {
                color = scale->logColor(doseMeter->integralDose());
            } else if(colorScale.is(LINER_SCALE)) {
                color = scale->linerColor(doseMeter->integralDose());
            }
            doseMeter->setColor(color);
            doseMeter->notifyStateChange();
        }
    }
}


void DoseSimulatorItemImpl::setDefaultShieldFile(const string& filename)
{
    defaultShieldTableFile = filename;
}


void DoseSimulatorItemImpl::onGammaDataLoaded()
{
    isLoaded = false;
    nodeData = nullptr;
    nodeData = crossSectionItem->nodeData();
    if(nodeData) {
        nodeData->createShieldData(defaultShieldTableFile, crossSectionItem->gammaData());
        isLoaded = true;
    }
}


bool DoseSimulatorItemImpl::onColorScalePropertyChanged(const int& index)
{
    colorScale.selectIndex(index);
    return true;
}


Item* DoseSimulatorItem::doDuplicate() const
{
    return new DoseSimulatorItem(*this);
}
    

void DoseSimulatorItem::doPutProperties(PutPropertyFunction& putProperty)
{
    SubSimulatorItem::doPutProperties(putProperty);
    impl->doPutProperties(putProperty);
}


void DoseSimulatorItemImpl::doPutProperties(PutPropertyFunction& putProperty)
{
    putProperty(_("ColorScale"), colorScale,
                [&](int index){ return onColorScalePropertyChanged(index); });
    FilePathProperty shieldFileProperty(
                defaultShieldTableFile, { _("Shield definition file (*.yaml)") });
    putProperty(_("Default shield table"), shieldFileProperty,
                  [&](const string& filename){ setDefaultShieldFile(filename); return true; });
}


bool DoseSimulatorItem::store(Archive& archive)
{
    SubSimulatorItem::store(archive);
    return impl->store(archive);
}


bool DoseSimulatorItemImpl::store(Archive& archive)
{
    archive.write("color_scale", colorScale.selectedIndex());
    archive.writeRelocatablePath("default_shield_table_file", defaultShieldTableFile);
    return true;
}


bool DoseSimulatorItem::restore(const Archive& archive)
{
    SubSimulatorItem::restore(archive);
    return impl->restore(archive);
}


bool DoseSimulatorItemImpl::restore(const Archive& archive)
{
    colorScale.selectIndex(archive.get("color_scale", 0));
    archive.readRelocatablePath("default_shield_table_file", defaultShieldTableFile);
    return true;
}
