/**
   @author Kenta Suzuki
*/

#include "DoseSimulatorItem.h"
#include <cnoid/Archive>
#include <cnoid/ExecutablePath>
#include <cnoid/Format>
#include <cnoid/ItemManager>
#include <cnoid/MessageView>
#include <cnoid/PutPropertyFunction>
#include <cnoid/Selection>
#include <cnoid/SimulatorItem>
#include <cnoid/UTF8>
#include <cnoid/WorldItem>
#include <map>
#include "ColorScale.h"
#include "CrossSectionItem.h"
#include "DoseMeter.h"
#include "GammaData.h"
#include "gettext.h"

#ifndef _MSC_VER
#include <alloca.h>
#endif

using namespace std;
using namespace cnoid;

namespace cnoid {

class DoseSimulatorItem::Impl
{
public:
    DoseSimulatorItem* self;

    Impl(DoseSimulatorItem* self);
    Impl(DoseSimulatorItem* self, const Impl& org);

    DeviceList<DoseMeter> doseMeters;
    double worldTimeStep;
    double timeUnit;
    OrthoNodeDataPtr nodeData;
    string defaultShieldTableFile;
    CrossSectionItem* crossSectionItem;
    bool isLoaded;

    Selection colorScale;

    enum ColorScaleId { LOG_SCALE, LINER_SCALE };

    bool initializeSimulation(SimulatorItem* simulatorItem);
    void onMidDynamics();
    void onPostDynamics();
    void setDefaultShieldFile(const string& filename);
    bool onColorScalePropertyChanged(const int& index);
    void onGammaDataLoaded();
};

}


void DoseSimulatorItem::initializeClass(ExtensionManager* ext)
{
    ext->itemManager()
        .registerClass<DoseSimulatorItem, SubSimulatorItem>(N_("DoseSimulatorItem"))
        .addCreationPanel<DoseSimulatorItem>();
}


DoseSimulatorItem::DoseSimulatorItem()
    : SubSimulatorItem()
{
    impl = new Impl(this);
}


DoseSimulatorItem::Impl::Impl(DoseSimulatorItem* self)
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
    impl = new Impl(this, *org.impl);
}


DoseSimulatorItem::Impl::Impl(DoseSimulatorItem* self, const Impl& org)
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


bool DoseSimulatorItem::Impl::initializeSimulation(SimulatorItem* simulatorItem)
{
    worldTimeStep = simulatorItem->worldTimeStep();
    doseMeters.clear();
    crossSectionItem = nullptr;

    const vector<SimulationBody*>& simBodies = simulatorItem->simulationBodies();
    for(auto& simBody : simBodies) {
        Body* body = simBody->body();
        doseMeters << body->devices();
    }

    OrthoNodeData::clearShield();
    for(auto& doseMeter : doseMeters) {
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
            simulatorItem->addMidDynamicsFunction([&](){ onMidDynamics(); });
            simulatorItem->addPostDynamicsFunction([&](){ onPostDynamics(); });
            crossSectionItem->sigGammaDataLoaded().connect([&](){ onGammaDataLoaded(); });
        }
    } else {
        MessageView::instance()->putln(formatR(_("GammaData was not found.")));
        return false;
    }

    return true;
}


void DoseSimulatorItem::Impl::onMidDynamics()
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


void DoseSimulatorItem::Impl::onPostDynamics()
{
    if(doseMeters.size()) {
        vector<double> integralDoses;
        for(auto& doseMeter : doseMeters) {
            integralDoses.push_back(doseMeter->integralDose());
        }

        double min = *min_element(integralDoses.begin(), integralDoses.end());
        double max = *max_element(integralDoses.begin(), integralDoses.end());
        ColorScale* scale = new ColorScale();
        int exp = (int)floor(log10(fabs(max))) + 1;
        min = 1.0 * pow(10, exp - 6);
        max = 1.0 * pow(10, exp);
        scale->setRange(min, max);

        for(auto& doseMeter : doseMeters) {
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


void DoseSimulatorItem::Impl::setDefaultShieldFile(const string& filename)
{
    defaultShieldTableFile = filename;
}


void DoseSimulatorItem::Impl::onGammaDataLoaded()
{
    isLoaded = false;
    nodeData = nullptr;
    nodeData = crossSectionItem->nodeData();
    if(nodeData) {
        nodeData->createShieldData(defaultShieldTableFile, crossSectionItem->gammaData());
        isLoaded = true;
    }
}


bool DoseSimulatorItem::Impl::onColorScalePropertyChanged(const int& index)
{
    colorScale.selectIndex(index);
    return true;
}


Item* DoseSimulatorItem::doCloneItem(CloneMap* cloneMap) const
{
    return new DoseSimulatorItem(*this);
}


void DoseSimulatorItem::doPutProperties(PutPropertyFunction& putProperty)
{
    SubSimulatorItem::doPutProperties(putProperty);
    putProperty(_("ColorScale"), impl->colorScale,
                [&](int index){ return impl->onColorScalePropertyChanged(index); });
    FilePathProperty shieldFileProperty(
                impl->defaultShieldTableFile, { _("Shield definition file (*.yaml)") });
    putProperty(_("Default shield table"), shieldFileProperty,
                  [&](const string& filename){ impl->setDefaultShieldFile(filename); return true; });
}


bool DoseSimulatorItem::store(Archive& archive)
{
    if(!SubSimulatorItem::store(archive)) {
        return false;
    }
    archive.write("color_scale", impl->colorScale.selectedIndex());
    archive.writeRelocatablePath("default_shield_table_file", impl->defaultShieldTableFile);
    return true;
}


bool DoseSimulatorItem::restore(const Archive& archive)
{
    if(!SubSimulatorItem::restore(archive)) {
        return false;
    }
    impl->colorScale.selectIndex(archive.get("color_scale", 0));
    archive.readRelocatablePath("default_shield_table_file", impl->defaultShieldTableFile);
    return true;
}
