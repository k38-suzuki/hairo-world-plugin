/**
   \file
   \author Kenta Suzuki
*/

#include "FluidAreaItem.h"
#include <cnoid/Archive>
#include <cnoid/EigenArchive>
#include <cnoid/EigenTypes>
#include <cnoid/EigenUtil>
#include <cnoid/FloatingNumberString>
#include <cnoid/ItemManager>
#include <cnoid/MeshGenerator>
#include <cnoid/PutPropertyFunction>
#include <cnoid/Selection>
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
    double d = 0.0;
    if(node.read("name", s)) item->setName(s);
    if(read(node, "translation", v)) item->setTranslation(v);
    if(read(node, "rotation", v)) item->setRotation(v);
    if(node.read("density", d)) item->setDensity(d);
    if(node.read("viscosity", d)) item->setViscosity(d);
    if(node.read("type", s)) item->setType(s);
    if(read(node, "size", v)) item->setSize(v);
    if(node.read("radius", d)) item->setRadius(d);
    if(node.read("height", d)) item->setHeight(d);
    if(read(node, "flow", v)) item->setFlow(v);

    if(!item->type().empty()) {
        if(read(node, "diffuseColor", v)) item->setDiffuseColor(v);
        if(read(node, "emissiveColor", v)) item->setEmissiveColor(v);
        if(read(node, "specularColor", v)) item->setSpecularColor(v);
        if(node.read("shininess", d)) item->setShininess(d);
        double t = 0.9;
        if(node.read("transparency", t)) item->setTransparency(t);
        item->updateScene();
    }
}


bool loadDocument(const string& fileName)
{
    YAMLReader reader;
    if(reader.load(fileName)) {
        Mapping* topNode = reader.loadDocument(fileName)->toMapping();
        Listing* fluidList = topNode->findListing("fluids");
        if(fluidList->isValid()) {
            for(int i = 0; i < fluidList->size(); i++) {
                FluidAreaItem* item = new FluidAreaItem();
                Mapping* info = fluidList->at(i)->toMapping();
                loadItem(*info, item);
            }
        }
    }
    return true;
}


bool loadDocument(FluidAreaItem* item, const string& fileName)
{
    YAMLReader reader;
    if(reader.load(fileName)) {
        Mapping* topNode = reader.loadDocument(fileName)->toMapping();
        Listing* fluidList = topNode->findListing("fluids");
        if(fluidList->isValid()) {
            for(int i = 0; i < fluidList->size(); i++) {
                Mapping* info = fluidList->at(i)->toMapping();
                loadItem(*info, item);
            }
        }
    }
    return true;
}


void putKeyVector3(YAMLWriter* writer, const string& key, const Vector3& value)
{
    writer->putKey(key);
    writer->startFlowStyleListing();
    for(int i = 0; i < 3; i++) {
        writer->putScalar(value[i]);
    }
    writer->endListing();
}

}


namespace cnoid {

class FluidAreaItemImpl
{
public:
    FluidAreaItemImpl(FluidAreaItem* self);
    FluidAreaItemImpl(FluidAreaItem* self, const FluidAreaItemImpl& org);

    FluidAreaItem* self;
    Vector3 translation;
    Vector3 rotation;
    FloatingNumberString density;
    FloatingNumberString viscosity;
    Selection type;
    Vector3 size;
    FloatingNumberString radius;
    FloatingNumberString height;
    Vector3 flow;
    Vector3 diffuseColor;
    Vector3 emissiveColor;
    Vector3 specularColor;
    FloatingNumberString shininess;
    FloatingNumberString transparency;
    SgPosTransformPtr scene;

    void doPutProperties(PutPropertyFunction& putProperty);
    bool store(Archive& archive);
    bool restore(const Archive& archive);
    void generateShape();
    void updateScene();
    bool onTranslationPropertyChanged(const string& value);
    bool onRotationPropertyChanged(const string& value);
    bool onAreaTypePropertyChanged(const int& index);
    bool onAreaSizePropertyChanged(const string& value);
    bool onAreaRadiusPropertyChanged(const string& value);
    bool onAreaHeightPropertyChanged(const string& value);
    bool onDiffuseColorPropertyChanged(const string& value);
    bool onEmissiveColorPropertyChanged(const string& value);
    bool onSpecularColorPropertyChanged(const string& value);
    bool onShininessPropertyChanged(const string& value);
    bool onTransparencyPropertyChanged(const string& value);
};

}


FluidAreaItem::FluidAreaItem()
{
    impl = new FluidAreaItemImpl(this);
}


FluidAreaItemImpl::FluidAreaItemImpl(FluidAreaItem* self)
    : self(self)
{
    translation << 0.0, 0.0, 0.0;
    rotation << 0.0, 0.0, 0.0;
    density = 0.0;
    viscosity = 0.0;
    type.setSymbol(FluidAreaItem::BOX, N_("Box"));
    type.setSymbol(FluidAreaItem::CYLINDER, N_("Cylinder"));
    type.setSymbol(FluidAreaItem::SPHERE, N_("Sphere"));
    size << 1.0, 1.0, 1.0;
    radius = 0.5;
    height = 1.0;
    flow << 0.0, 0.0, 0.0;
    diffuseColor << 0.0, 1.0, 1.0;
    emissiveColor << 0.0, 0.0, 0.0;
    specularColor << 0.0, 0.0, 0.0;
    shininess = 0.0;
    transparency = 0.8;
    scene = new SgPosTransform();
    generateShape();
    updateScene();
}


FluidAreaItem::FluidAreaItem(const FluidAreaItem& org)
    : Item(org),
      impl(new FluidAreaItemImpl(this, *org.impl))
{

}


FluidAreaItemImpl::FluidAreaItemImpl(FluidAreaItem* self, const FluidAreaItemImpl& org)
    : self(self)
{
    translation = org.translation;
    rotation = org.rotation;
    density = org.density;
    viscosity = org.viscosity;
    type = org.type;
    size = org.size;
    radius = org.radius;
    height = org.height;
    flow = org.flow;
    diffuseColor = org.diffuseColor;
    emissiveColor = org.emissiveColor;
    specularColor = org.specularColor;
    shininess = org.shininess;
    transparency = org.transparency;
    scene = new SgPosTransform();
    generateShape();
    updateScene();
}


FluidAreaItem::~FluidAreaItem()
{
    delete impl;
}


SgNode* FluidAreaItem::getScene()
{
    return impl->scene;
}


void FluidAreaItem::initializeClass(ExtensionManager* ext)
{
    ItemManager& im = ext->itemManager();
    im.registerClass<FluidAreaItem>(N_("FluidAreaItem"));
    im.addCreationPanel<FluidAreaItem>();

    im.addLoaderAndSaver<FluidAreaItem>(
        _("Fluid Area"), "FLUID-AREA-FILE", "yaml;yml",
        [](FluidAreaItem* item, const std::string& filename, std::ostream& os, Item*){ return load(item, filename); },
        [](FluidAreaItem* item, const std::string& filename, std::ostream& os, Item*){ return save(item, filename); },
        ItemManager::PRIORITY_CONVERSION);
}


void FluidAreaItem::setTranslation(const Vector3& translation)
{
    impl->translation = translation;
}


Vector3 FluidAreaItem::translation() const
{
    return impl->translation;
}


void FluidAreaItem::setRotation(const Vector3& rotation)
{
    impl->rotation = rotation;
}


Vector3 FluidAreaItem::rotation() const
{
    return impl->rotation;
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


void FluidAreaItem::setType(const string& type)
{
    impl->type.select(type);
}


string FluidAreaItem::type() const
{
    return impl->type.symbol(impl->type.selectedIndex());
}


void FluidAreaItem::setSize(const Vector3& size)
{
    impl->size = size;
}


Vector3 FluidAreaItem::size() const
{
    return impl->size;
}


void FluidAreaItem::setRadius(const double& radius)
{
    impl->radius = radius;
}


double FluidAreaItem::radius() const
{
    return impl->radius.value();
}


void FluidAreaItem::setHeight(const double& height)
{
    impl->height = height;
}


double FluidAreaItem::height() const
{
    return impl->height.value();
}


void FluidAreaItem::setFlow(const Vector3& flow)
{
    impl->flow = flow;
}


Vector3 FluidAreaItem::flow() const
{
    return impl->flow;
}


void FluidAreaItem::setDiffuseColor(const Vector3& diffuseColor)
{
    impl->diffuseColor = diffuseColor;
}


Vector3 FluidAreaItem::diffuseColor() const
{
    return impl->diffuseColor;
}


void FluidAreaItem::setEmissiveColor(const Vector3& emissiveColor)
{
    impl->emissiveColor = emissiveColor;
}


Vector3 FluidAreaItem::emissiveColor() const
{
    return impl->emissiveColor;
}


void FluidAreaItem::setSpecularColor(const Vector3& specularColor)
{
    impl->specularColor = specularColor;
}


Vector3 FluidAreaItem::specularColor() const
{
    return impl->specularColor;
}


void FluidAreaItem::setShininess(const double& shininess)
{
    impl->shininess = shininess;
}


double FluidAreaItem::shininess() const
{
    return impl->shininess.value();
}


void FluidAreaItem::setTransparency(const double& transparency)
{
    impl->transparency = transparency;
}


double FluidAreaItem::transparency() const
{
    return impl->transparency.value();
}


bool FluidAreaItemImpl::onTranslationPropertyChanged(const string& value)
{
    Vector3 translation;
    if(toVector3(value, translation)) {
        this->translation = translation;
        scene->setTranslation(translation);
        scene->notifyUpdate();
        return true;
    }
    return false;
}


bool FluidAreaItemImpl::onRotationPropertyChanged(const string& value)
{
    Vector3 rotation;
    if(toVector3(value, rotation)) {
        this->rotation = rotation;
        scene->setRotation(rotFromRpy(rotation * TO_RADIAN));
        scene->notifyUpdate();
        return true;
    }
    return false;
}


bool FluidAreaItemImpl::onAreaTypePropertyChanged(const int& index)
{
    MeshGenerator generator;

    type.selectIndex(index);
    SgMesh* mesh;
    if(type.is(FluidAreaItem::BOX)) {
        mesh = generator.generateBox(size);
    } else if(type.is(FluidAreaItem::CYLINDER)) {
        mesh = generator.generateCylinder(radius.value(), height.value());
    } else if(type.is(FluidAreaItem::SPHERE)) {
        mesh = generator.generateSphere(radius.value());
    }
    SgShape* shape = dynamic_cast<SgShape*>(scene->child(0));
    shape->setMesh(mesh);
    shape->notifyUpdate();
    return true;
}


bool FluidAreaItemImpl::onAreaSizePropertyChanged(const string& value)
{
    Vector3 size;
    MeshGenerator generator;

    if(toVector3(value, size)) {
        this->size = size;
        SgMesh* mesh = generator.generateBox(size);
        SgShape* shape = dynamic_cast<SgShape*>(scene->child(0));
        shape->setMesh(mesh);
        shape->notifyUpdate();
        return true;
    }
    return false;
}


bool FluidAreaItemImpl::onAreaRadiusPropertyChanged(const string& value)
{
    double radius = stod(value);
    MeshGenerator generator;

    if(radius >= 0) {
        this->radius = radius;
        SgMesh* mesh;
        if(type.is(FluidAreaItem::CYLINDER)) {
            mesh = generator.generateCylinder(radius, height.value());
        } else if(type.is(FluidAreaItem::SPHERE)) {
            mesh = generator.generateSphere(radius);
        }
        SgShape* shape = dynamic_cast<SgShape*>(scene->child(0));
        shape->setMesh(mesh);
        shape->notifyUpdate();
        return true;
    }
    return false;
}


bool FluidAreaItemImpl::onAreaHeightPropertyChanged(const string& value)
{
    double height = stod(value);
    MeshGenerator generator;

    if(height >= 0) {
        this->height = height;
        SgMesh* mesh = generator.generateCylinder(radius.value(), height);
        SgShape* shape = dynamic_cast<SgShape*>(scene->child(0));
        shape->setMesh(mesh);
        shape->notifyUpdate();
        return true;
    }
    return false;
}


bool FluidAreaItemImpl::onDiffuseColorPropertyChanged(const string& value)
{
    Vector3 diffuseColor;
    if(toVector3(value, diffuseColor)) {
        this->diffuseColor = diffuseColor;
        SgShape* shape = dynamic_cast<SgShape*>(scene->child(0));
        SgMaterial* material = shape->material();
        material->setDiffuseColor(diffuseColor);
        material->notifyUpdate();
        return true;
    }
    return false;
}


bool FluidAreaItemImpl::onEmissiveColorPropertyChanged(const string& value)
{
    Vector3 emissiveColor;
    if(toVector3(value, emissiveColor)) {
        this->emissiveColor = emissiveColor;
        SgShape* shape = dynamic_cast<SgShape*>(scene->child(0));
        SgMaterial* material = shape->material();
        material->setEmissiveColor(emissiveColor);
        material->notifyUpdate();
        return true;
    }
    return false;
}


bool FluidAreaItemImpl::onSpecularColorPropertyChanged(const string& value)
{
    Vector3 specularColor;
    if(toVector3(value, specularColor)) {
        this->specularColor = specularColor;
        SgShape* shape = dynamic_cast<SgShape*>(scene->child(0));
        SgMaterial* material = shape->material();
        material->setSpecularColor(specularColor);
        material->notifyUpdate();
        return true;
    }
    return false;
}


bool FluidAreaItemImpl::onShininessPropertyChanged(const string& value)
{
    float shininess = stof(value);
    float s = 127.0f * std::max(0.0f, std::min(shininess, 1.0f)) + 1.0f;
    if(shininess >= 0) {
        this->shininess = shininess;
        SgShape* shape = dynamic_cast<SgShape*>(scene->child(0));
        SgMaterial* material = shape->material();
        material->setSpecularExponent(s);
        material->notifyUpdate();
        return true;
    }
    return false;
}


bool FluidAreaItemImpl::onTransparencyPropertyChanged(const string& value)
{
    double transparency = stod(value);
    if(transparency >= 0) {
        this->transparency = transparency;
        SgShape* shape = dynamic_cast<SgShape*>(scene->child(0));
        SgMaterial* material = shape->material();
        material->setTransparency(transparency);
        material->notifyUpdate();
        return true;
    }
    return false;
}


void FluidAreaItemImpl::generateShape()
{
    SgShape* shape = new SgShape();
    SgMaterial* material = new SgMaterial();
    shape->setMaterial(material);
    scene->addChild(shape, true);
}


void FluidAreaItem::updateScene()
{
    impl->updateScene();
}


void FluidAreaItemImpl::updateScene()
{
    MeshGenerator generator;

    scene->setTranslation(translation);
    if(type.is(FluidAreaItem::CYLINDER)) {
        scene->setRotation(rotFromRpy(rotation * TO_RADIAN));
    }
    SgMesh* mesh;
    if(type.is(FluidAreaItem::BOX)) {
        mesh = generator.generateBox(size);
    } else if(type.is(FluidAreaItem::CYLINDER)) {
        mesh = generator.generateCylinder(radius.value(), height.value());
    } else if(type.is(FluidAreaItem::SPHERE)) {
        mesh = generator.generateSphere(radius.value());
    }
    SgShape* shape = dynamic_cast<SgShape*>(scene->child(0));
    SgMaterial* material = shape->material();
    float s = 127.0f * std::max(0.0f, std::min((float)shininess.value(), 1.0f)) + 1.0f;
    shape->setMesh(mesh);
    material->setDiffuseColor(diffuseColor);
    material->setEmissiveColor(emissiveColor);
    material->setSpecularColor(specularColor);
    material->setSpecularExponent(s);
    material->setTransparency(transparency.value());
    shape->setMaterial(material);
    shape->notifyUpdate();
}


bool FluidAreaItem::load(FluidAreaItem* item, const string& fileName)
{
    if(!loadDocument(item, fileName)) {
        return false;
    }
    return true;
}


bool FluidAreaItem::save(FluidAreaItem* item, const string& fileName)
{
    if(!fileName.empty()) {
        YAMLWriter writer(fileName);

        writer.startMapping();
        writer.putKey("fluids");
        writer.startListing();

        writer.startMapping();
        writer.putKeyValue("name", item->name());
        putKeyVector3(&writer, "translation", item->translation());
        putKeyVector3(&writer, "rotation", item->rotation());
        writer.putKeyValue("type", item->type());
        if(item->type() == "Box") {
            putKeyVector3(&writer, "size", item->size());
        } else if(item->type() == "Cylinder") {
            writer.putKeyValue("radius", item->radius());
            writer.putKeyValue("height", item->height());
        } else if(item->type() == "Sphere") {
            writer.putKeyValue("radius", item->radius());
        }
        writer.putKeyValue("density", item->density());
        writer.putKeyValue("visocosity", item->viscosity());
        putKeyVector3(&writer, "flow", item->flow());
        putKeyVector3(&writer, "diffuseColor", item->diffuseColor());
        putKeyVector3(&writer, "emissiveColor", item->emissiveColor());
        putKeyVector3(&writer, "specularColor", item->specularColor());
        writer.putKeyValue("shininess", item->shininess());
        writer.putKeyValue("transparency", item->transparency());
        writer.endMapping();

        writer.endListing();
        writer.endMapping();
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
}


void FluidAreaItemImpl::doPutProperties(PutPropertyFunction& putProperty)
{
    putProperty(_("Density"), density,
                [&](const string& v){ return density.setNonNegativeValue(v); });
    putProperty(_("Viscosity"), viscosity,
                [&](const string& v){ return viscosity.setNonNegativeValue(v); });
    putProperty(_("Flow"), str(flow), [&](const string& v){ return toVector3(v, flow); });
    putProperty(_("Shape"), type,
                [&](int index){ return onAreaTypePropertyChanged(index); });
    if(type.is(FluidAreaItem::BOX)) {
        putProperty(_("Size"), str(size),
                [&](const string& value){ return onAreaSizePropertyChanged(value); });
    } else if(type.is(FluidAreaItem::CYLINDER) || type.is(FluidAreaItem::SPHERE)) {
        putProperty(_("Radius"), to_string(radius.value()),
                [&](const string& value){ return onAreaRadiusPropertyChanged(value); });
        if(type.is(FluidAreaItem::CYLINDER)) {
            putProperty(_("Height"), to_string(height.value()),
                    [&](const string& value){ return onAreaHeightPropertyChanged(value); });
        }
    }
    putProperty(_("Translation"), str(translation),
            [&](const string& value){ return onTranslationPropertyChanged(value); });
    if(type.is(FluidAreaItem::CYLINDER)) {
        putProperty(_("RPY"), str(rotation),
                [&](const string& value){ return onRotationPropertyChanged(value); });
    }
    putProperty(_("DiffuseColor"), str(diffuseColor),
            [&](const string& value){ return onDiffuseColorPropertyChanged(value); });
//    putProperty(_("EmissiveColor"), str(emissiveColor),
//            [&](const string& value){ return onEmissiveColorPropertyChanged(value); });
//    putProperty(_("SpecularColor"), str(specularColor),
//            [&](const string& value){ return onSpecularColorPropertyChanged(value); });
//    putProperty(_("Shininess"), to_string(shininess.value()),
//            [&](const string& value){ return onShininessPropertyChanged(value); });
    putProperty(_("Transparency"), to_string(transparency.value()),
            [&](const string& value){ return onTransparencyPropertyChanged(value); });
}


bool FluidAreaItem::store(Archive& archive)
{
    return impl->store(archive);
}


bool FluidAreaItemImpl::store(Archive& archive)
{
    write(archive, "translation", translation);
    write(archive, "rotation", rotation);
    archive.write("density", density);
    archive.write("viscosity", viscosity);
    archive.write("type", type.selectedSymbol(), DOUBLE_QUOTED);
    write(archive, "size", size);
    archive.write("radius", radius);
    archive.write("height", height);
    write(archive, "flow", flow);
    write(archive, "diffuseColor", diffuseColor);
    write(archive, "emissiveColor", emissiveColor);
    write(archive, "specularColor", specularColor);
    archive.write("shininess", shininess);
    archive.write("transparency", transparency);
    return true;
}


bool FluidAreaItem::restore(const Archive &archive)
{
    return impl->restore(archive);
}


bool FluidAreaItemImpl::restore(const Archive& archive)
{
    read(archive, "translation", translation);
    read(archive, "rotation", rotation);
    density = archive.get("density", density.string());
    viscosity = archive.get("viscosity", viscosity.string());
    string symbol;
    if(archive.read("type", symbol)) {
        type.select(symbol);
    }
    read(archive, "size", size);
    radius = archive.get("radius", radius.string());
    height = archive.get("height", height.string());
    read(archive, "flow", flow);
    read(archive, "diffuseColor", diffuseColor);
    read(archive, "emissiveColor", emissiveColor);
    read(archive, "specularColor", specularColor);
    shininess = archive.get("shininess", shininess.string());
    transparency = archive.get("transparency", transparency.string());
    updateScene();
    return true;
}
