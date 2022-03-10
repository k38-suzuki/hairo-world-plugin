/**
   \file
   \author Kenta Suzuki
*/

#include "FluidAreaItem.h"
#include <cnoid/Archive>
#include <cnoid/EigenArchive>
#include <cnoid/FloatingNumberString>
#include <cnoid/ItemManager>
#include <cnoid/MessageView>
#include <cnoid/PutPropertyFunction>
#include <cnoid/YAMLReader>
#include <cnoid/YAMLWriter>
#include "gettext.h"

using namespace std;
using namespace cnoid;

namespace {

void loadItem(Mapping& node, FluidAreaItem* item)
{
    string s;
    Vector3 v;
    int i;
    double d = 0.0;

    if(node.read("density", d)) item->setDensity(d);
    if(node.read("viscosity", d)) item->setViscosity(d);
    if(read(node, "flow", v)) item->setFlow(v);

    if(node.read("name", s)) item->setName(s);
    if(read(node, "translation", v)) item->setTranslation(v);
    if(read(node, "rotation", v)) item->setRotation(v);
    if(node.read("type", i)) item->setType(i);
    if(read(node, "size", v)) item->setSize(v);
    if(node.read("radius", d)) item->setRadius(d);
    if(node.read("height", d)) item->setHeight(d);
    if(read(node, "diffuseColor", v)) item->setDiffuseColor(v);
    if(read(node, "emissiveColor", v)) item->setEmissiveColor(v);
    if(read(node, "specularColor", v)) item->setSpecularColor(v);
    if(node.read("shininess", d)) item->setShininess(d);
    double t = 0.9;
    if(node.read("transparency", t)) item->setTransparency(t);
    item->updateScene();
}


bool loadDocument(const string& filename)
{
    try {
        YAMLReader reader;
        MappingPtr node = reader.loadDocument(filename)->toMapping();
        if(node) {
            auto& fluidList = *node->findListing("fluids");
            if(fluidList.isValid()) {
                for(int i = 0; i < fluidList.size(); i++) {
                    FluidAreaItem* item = new FluidAreaItem;
                    Mapping* info = fluidList[i].toMapping();
                    loadItem(*info, item);
                }
            }
        }
    }
    catch (const ValueNode::Exception& ex) {
        MessageView::instance()->putln(ex.message());
    }

    return true;
}


bool loadDocument(FluidAreaItem* item, const string& filename)
{
    try {
        YAMLReader reader;
        MappingPtr node = reader.loadDocument(filename)->toMapping();
        auto& fluidList = *node->findListing("fluids");
        if(fluidList.isValid()) {
            for(int i = 0; i < fluidList.size(); i++) {
                Mapping* info = fluidList[i].toMapping();
                loadItem(*info, item);
            }
        }
    }
    catch (const ValueNode::Exception& ex) {
        MessageView::instance()->putln(ex.message());
    }

    return true;
}


void putKeyVector3(YAMLWriter& writer, const string& key, const Vector3& value)
{
    writer.putKey(key);
    writer.startFlowStyleListing(); {
        for(int i = 0; i < 3; ++i) {
            writer.putScalar(value[i]);
        }
    } writer.endListing();
}

}


namespace cnoid {

class FluidAreaItemImpl
{
public:
    FluidAreaItemImpl(FluidAreaItem* self);
    FluidAreaItemImpl(FluidAreaItem* self, const FluidAreaItemImpl& org);
    FluidAreaItem* self;

    FloatingNumberString density;
    FloatingNumberString viscosity;
    Vector3 flow;

    void doPutProperties(PutPropertyFunction& putProperty);
    bool store(Archive& archive);
    bool restore(const Archive& archive);
};

}


FluidAreaItem::FluidAreaItem()
{
    impl = new FluidAreaItemImpl(this);
}


FluidAreaItemImpl::FluidAreaItemImpl(FluidAreaItem* self)
    : self(self)
{
    density = 0.0;
    viscosity = 0.0;
    flow << 0.0, 0.0, 0.0;
}


FluidAreaItem::FluidAreaItem(const FluidAreaItem& org)
    : AreaItem(org),
      impl(new FluidAreaItemImpl(this, *org.impl))
{

}


FluidAreaItemImpl::FluidAreaItemImpl(FluidAreaItem* self, const FluidAreaItemImpl& org)
    : self(self)
{
    density = org.density;
    viscosity = org.viscosity;
    flow = org.flow;
}


FluidAreaItem::~FluidAreaItem()
{
    delete impl;
}


void FluidAreaItem::initializeClass(ExtensionManager* ext)
{
    ItemManager& im = ext->itemManager();
    im.registerClass<FluidAreaItem>(N_("FluidAreaItem"));
    im.addCreationPanel<FluidAreaItem>();

//    im.addLoaderAndSaver<FluidAreaItem>(
//        _("Fluid Area"), "FLUID-AREA-FILE", "yaml;yml",
//        [](FluidAreaItem* item, const std::string& filename, std::ostream& os, Item*){ return load(item, filename); },
//        [](FluidAreaItem* item, const std::string& filename, std::ostream& os, Item*){ return save(item, filename); },
//        ItemManager::PRIORITY_CONVERSION);
}


void FluidAreaItem::setDensity(const double& density)
{
    impl->density = density;
}


double FluidAreaItem::density() const
{
    return impl->density.value();
}


void FluidAreaItem::setViscosity(const double& viscosity)
{
    impl->viscosity = viscosity;
}


double FluidAreaItem::viscosity() const
{
    return impl->viscosity.value();
}


void FluidAreaItem::setFlow(const Vector3& flow)
{
    impl->flow = flow;
}


Vector3 FluidAreaItem::flow() const
{
    return impl->flow;
}


bool FluidAreaItem::load(FluidAreaItem* item, const string& filename)
{
    if(!loadDocument(item, filename)) {
        return false;
    }
    return true;
}


bool FluidAreaItem::save(FluidAreaItem* item, const string& filename)
{
    if(!filename.empty()) {
        YAMLWriter writer(filename);
        writer.startMapping(); {
            writer.putKey("fluids");
            writer.startListing(); {
                writer.startMapping(); {
                    writer.putKeyValue("name", item->name());
                    putKeyVector3(writer, "translation", item->translation());
                    putKeyVector3(writer, "rotation", item->rotation());
                    writer.putKeyValue("type", item->type());
                    if(item->type() == AreaItem::BOX) {
                        putKeyVector3(writer, "size", item->size());
                    } else if(item->type() == AreaItem::CYLINDER) {
                        writer.putKeyValue("radius", item->radius());
                        writer.putKeyValue("height", item->height());
                    } else if(item->type() == AreaItem::SPHERE) {
                        writer.putKeyValue("radius", item->radius());
                    }
                    writer.putKeyValue("density", item->density());
                    writer.putKeyValue("visocosity", item->viscosity());
                    putKeyVector3(writer, "flow", item->flow());
                    putKeyVector3(writer, "diffuseColor", item->diffuseColor());
                    putKeyVector3(writer, "emissiveColor", item->emissiveColor());
                    putKeyVector3(writer, "specularColor", item->specularColor());
                    writer.putKeyValue("shininess", item->shininess());
                    writer.putKeyValue("transparency", item->transparency());
                } writer.endMapping();
            } writer.endListing();
        } writer.endMapping();
    }
    return true;
}


Item* FluidAreaItem::doDuplicate() const
{
    return new FluidAreaItem(*this);
}


void FluidAreaItem::doPutProperties(PutPropertyFunction& putProperty)
{
    impl->doPutProperties(putProperty);
    AreaItem::doPutProperties(putProperty);
}


void FluidAreaItemImpl::doPutProperties(PutPropertyFunction& putProperty)
{
    putProperty(_("Density"), density,
                [&](const string& v){ return density.setNonNegativeValue(v); });
    putProperty(_("Viscosity"), viscosity,
                [&](const string& v){ return viscosity.setNonNegativeValue(v); });
    putProperty(_("Flow"), str(flow), [&](const string& v){ return toVector3(v, flow); });
}


bool FluidAreaItem::store(Archive& archive)
{
    AreaItem::store(archive);
    return impl->store(archive);
}


bool FluidAreaItemImpl::store(Archive& archive)
{
    archive.write("density", density);
    archive.write("viscosity", viscosity);
    write(archive, "flow", flow);
    return true;
}


bool FluidAreaItem::restore(const Archive &archive)
{
    AreaItem::restore(archive);
    return impl->restore(archive);
}


bool FluidAreaItemImpl::restore(const Archive& archive)
{
    density = archive.get("density", density.string());
    viscosity = archive.get("viscosity", viscosity.string());
    read(archive, "flow", flow);
    return true;
}
