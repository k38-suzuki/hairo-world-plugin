/**
   \file
   \author Kenta Suzuki
*/

#include "TCAreaItem.h"
#include <cnoid/Archive>
#include <cnoid/EigenArchive>
#include <cnoid/EigenUtil>
#include <cnoid/FloatingNumberString>
#include <cnoid/ItemManager>
#include <cnoid/MeshGenerator>
#include <cnoid/PutPropertyFunction>
#include <cnoid/RootItem>
#include <cnoid/SceneGraph>
#include <cnoid/Selection>
#include <cnoid/YAMLReader>
#include <cnoid/YAMLWriter>
#include "gettext.h"

using namespace std;
using namespace cnoid;

namespace {

void loadItem(Mapping& node, TCAreaItem* item)
{
    string s;
    Vector3 v;
    double d = 0.0;
    if(node.read("name", s)) item->setName(s);
    if(read(node, "translation", v)) item->setTranslation(v);
    if(read(node, "rotation", v)) item->setRotation(v);
    if(node.read("type", s)) item->setType(s);
    if(read(node, "size", v)) item->setSize(v);
    if(node.read("radius", d)) item->setRadius(d);
    if(node.read("height", d)) item->setHeight(d);
    if(node.read("inboundDelay", d)) item->setInboundDelay(d);
    if(node.read("inboundRate", d)) item->setInboundRate(d);
    if(node.read("inboundLoss", d)) item->setInboundLoss(d);
    if(node.read("outboundDelay", d)) item->setOutboundDelay(d);
    if(node.read("outboundRate", d)) item->setOutboundRate(d);
    if(node.read("outboundLoss", d)) item->setOutboundLoss(d);
    if(node.read("sourceIP", s)) item->setSource(s);
    if(node.read("destinationIP", s)) item->setDestination(s);

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
        Listing* trafficList = topNode->findListing("traffics");
        if(trafficList->isValid()) {
            for(int i = 0; i < trafficList->size(); i++) {
                TCAreaItem* item = new TCAreaItem();
                Mapping* node = trafficList->at(i)->toMapping();
                loadItem(*node, item);
            }
        }
    }
    return true;
}


bool loadDocument(TCAreaItem* item, const string& fileName)
{
    YAMLReader reader;
    if(reader.load(fileName)) {
        Mapping* topNode = reader.loadDocument(fileName)->toMapping();
        Listing* trafficList = topNode->findListing("traffics");
        if(trafficList->isValid()) {
            for(int i = 0; i < trafficList->size(); i++) {
                Mapping* node = trafficList->at(i)->toMapping();
                loadItem(*node, item);
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

class TCAreaItemImpl
{
public:
    TCAreaItemImpl(TCAreaItem* self);
    TCAreaItemImpl(TCAreaItem* self, const TCAreaItemImpl& org);

    TCAreaItem* self;
    int id_;
    Vector3 translation;
    Vector3 rotation;
    Selection type;
    Vector3 size;
    FloatingNumberString radius;
    FloatingNumberString height;
    FloatingNumberString inboundDelay;
    FloatingNumberString inboundRate;
    FloatingNumberString inboundLoss;
    FloatingNumberString outboundDelay;
    FloatingNumberString outboundRate;
    FloatingNumberString outboundLoss;
    string source;
    string destination;
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


TCAreaItem::TCAreaItem()
{
    impl = new TCAreaItemImpl(this);
}


TCAreaItemImpl::TCAreaItemImpl(TCAreaItem* self)
    : self(self)
{
    id_ = INT_MAX;
    translation << 0.0, 0.0, 0.0;
    rotation << 0.0, 0.0, 0.0;
    type.setSymbol(TCAreaItem::BOX, N_("Box"));
    type.setSymbol(TCAreaItem::CYLINDER, N_("Cylinder"));
    type.setSymbol(TCAreaItem::SPHERE, N_("Sphere"));
    size << 1.0, 1.0, 1.0;
    radius = 0.5;
    height = 1.0;
    inboundDelay = 0.0;
    inboundRate = 0.0;
    inboundLoss = 0.0;
    outboundDelay = 0.0;
    outboundRate = 0.0;
    outboundLoss = 0.0;
    source.clear();
    destination.clear();
    diffuseColor << 1.0, 1.0, 0.0;
    emissiveColor << 0.0, 0.0, 0.0;
    specularColor << 0.0, 0.0, 0.0;
    shininess = 0.0;
    transparency = 0.8;
    scene = new SgPosTransform();
    generateShape();
    updateScene();
}


TCAreaItem::TCAreaItem(const TCAreaItem& org)
    : Item(org),
      impl(new TCAreaItemImpl(this, *org.impl))
{

}


TCAreaItemImpl::TCAreaItemImpl(TCAreaItem* self, const TCAreaItemImpl& org)
    : self(self)
{
    id_ = org.id_;
    translation = org.translation;
    rotation = org.rotation;
    type = org.type;
    size = org.size;
    radius = org.radius;
    height = org.height;
    inboundDelay = org.inboundDelay;
    inboundRate = org.inboundRate;
    inboundLoss = org.inboundLoss;
    outboundDelay = org.outboundDelay;
    outboundRate = org.outboundRate;
    outboundLoss = org.outboundLoss;
    source = org.source;
    destination = org.destination;
    diffuseColor = org.diffuseColor;
    emissiveColor = org.emissiveColor;
    specularColor = org.specularColor;
    shininess = org.shininess;
    transparency = org.transparency;
    scene = new SgPosTransform();
    generateShape();
    updateScene();
}


TCAreaItem::~TCAreaItem()
{
    delete impl;
}


SgNode* TCAreaItem::getScene()
{
    return impl->scene;
}


void TCAreaItem::initializeClass(ExtensionManager* ext)
{
    ItemManager& im = ext->itemManager();

    im.registerClass<TCAreaItem>(N_("TCAreaItem"));
    im.addCreationPanel<TCAreaItem>();

    im.addLoaderAndSaver<TCAreaItem>(
        _("TC Area"), "TC-AREA-FILE", "yaml;yml",
        [](TCAreaItem* item, const string& filename, std::ostream& os, Item*){ return load(item, filename); },
        [](TCAreaItem* item, const string& filename, std::ostream& os, Item*){ return save(item, filename); },
        ItemManager::PRIORITY_CONVERSION);
}


bool TCAreaItem::load(TCAreaItem* item, const string& fileName)
{
    if(!loadDocument(item, fileName)) {
        return false;
    }
    return true;
}


bool TCAreaItem::save(TCAreaItem* item, const string& fileName)
{
    if(!fileName.empty()) {
        YAMLWriter writer(fileName);

        writer.startMapping();
        writer.putKey("traffics");
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
        writer.putKeyValue("inboundDelay", item->inboundDelay());
        writer.putKeyValue("inboundRate", item->inboundRate());
        writer.putKeyValue("inboundLoss", item->inboundLoss());
        writer.putKeyValue("outboundDelay", item->outboundDelay());
        writer.putKeyValue("outboundRate", item->outboundRate());
        writer.putKeyValue("outboundLoss", item->outboundLoss());

        writer.putKeyValue("sourceIP", item->source());
        writer.putKeyValue("destinationIP", item->destination());

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


void TCAreaItem::updateScene()
{
    impl->updateScene();
}


void TCAreaItemImpl::updateScene()
{
    MeshGenerator generator;
    scene->setTranslation(translation);
    if(type.is(TCAreaItem::CYLINDER)) {
        scene->setRotation(rotFromRpy(rotation * TO_RADIAN));
    }
    SgMesh* mesh;
    if(type.is(TCAreaItem::BOX)) {
        mesh = generator.generateBox(size);
    } else if(type.is(TCAreaItem::CYLINDER)) {
        mesh = generator.generateCylinder(radius.value(), height.value());
    } else if(type.is(TCAreaItem::SPHERE)) {
        mesh = generator.generateSphere(radius.value());
    }
    SgShape* shape = dynamic_cast<SgShape*>(scene->child(0));
    SgMaterial* material = shape->material();
    shape->setMesh(mesh);
    material->setDiffuseColor(diffuseColor);
    material->setEmissiveColor(emissiveColor);
    material->setSpecularColor(specularColor);
    material->setShininess(shininess.value());
    material->setTransparency(transparency.value());
    shape->setMaterial(material);
    shape->notifyUpdate();
}

void TCAreaItem::setId(const int& id)
{
    impl->id_ = id;
}


int TCAreaItem::id() const
{
    return impl->id_;
}


void TCAreaItem::setTranslation(const Vector3& translation)
{
    impl->translation = translation;
}


Vector3 TCAreaItem::translation() const
{
    return impl->translation;
}


void TCAreaItem::setRotation(const Vector3& rotation)
{
    impl->rotation = rotation;
}


Vector3 TCAreaItem::rotation() const
{
    return impl->rotation;
}


void TCAreaItem::setType(const string& type)
{
    impl->type.select(type);
}


string TCAreaItem::type() const
{
    return impl->type.symbol(impl->type.selectedIndex());
}


void TCAreaItem::setSize(const Vector3& size)
{
    impl->size = size;
}


Vector3 TCAreaItem::size() const
{
    return impl->size;
}


void TCAreaItem::setRadius(const double& radius)
{
    impl->radius = radius;
}


double TCAreaItem::radius() const
{
    return impl->radius.value();
}


void TCAreaItem::setHeight(const double& height)
{
    impl->height = height;
}


double TCAreaItem::height() const
{
    return impl->height.value();
}


void TCAreaItem::setInboundDelay(const double& inboundDelay)
{
    impl->inboundDelay = inboundDelay;
}


double TCAreaItem::inboundDelay() const
{
    return impl->inboundDelay.value();
}


void TCAreaItem::setInboundRate(const double& inboundRate)
{
    impl->inboundRate = inboundRate;
}


double TCAreaItem::inboundRate() const
{
    return impl->inboundRate.value();
}


void TCAreaItem::setInboundLoss(const double& inboundLoss)
{
    impl->inboundLoss = inboundLoss;
}


double TCAreaItem::inboundLoss() const
{
    return impl->inboundLoss.value();
}


void TCAreaItem::setOutboundDelay(const double& outboundDelay)
{
    impl->outboundDelay = outboundDelay;
}


double TCAreaItem::outboundDelay() const
{
    return impl->outboundDelay.value();
}


void TCAreaItem::setOutboundRate(const double& outboundRate)
{
    impl->outboundRate = outboundRate;
}


double TCAreaItem::outboundRate() const
{
    return impl->outboundRate.value();
}


void TCAreaItem::setOutboundLoss(const double& outboundLoss)
{
    impl->outboundLoss = outboundLoss;
}


double TCAreaItem::outboundLoss() const
{
    return impl->outboundLoss.value();
}


void TCAreaItem::setSource(const string& source)
{
    impl->source = source;
}


string TCAreaItem::source() const
{
    return impl->source;
}


void TCAreaItem::setDestination(const string& destination)
{
    impl->destination = destination;
}


string TCAreaItem::destination() const
{
    return impl->destination;
}


void TCAreaItem::setDiffuseColor(const Vector3& diffuseColor)
{
    impl->diffuseColor = diffuseColor;
}


Vector3 TCAreaItem::diffuseColor() const
{
    return impl->diffuseColor;
}


void TCAreaItem::setEmissiveColor(const Vector3& emissiveColor)
{
    impl->emissiveColor = emissiveColor;
}


Vector3 TCAreaItem::emissiveColor() const
{
    return impl->emissiveColor;
}


void TCAreaItem::setSpecularColor(const Vector3& specularColor)
{
    impl->specularColor = specularColor;
}


Vector3 TCAreaItem::specularColor() const
{
    return impl->specularColor;
}


void TCAreaItem::setShininess(const double& shininess)
{
    impl->shininess = shininess;
}


double TCAreaItem::shininess() const
{
    return impl->shininess.value();
}


void TCAreaItem::setTransparency(const double& transparency)
{
    impl->transparency = transparency;
}


double TCAreaItem::transparency() const
{
    return impl->transparency.value();
}


bool TCAreaItemImpl::onTranslationPropertyChanged(const string& value)
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


bool TCAreaItemImpl::onRotationPropertyChanged(const string& value)
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


bool TCAreaItemImpl::onAreaTypePropertyChanged(const int& index)
{
    MeshGenerator generator;

    type.selectIndex(index);
    SgMesh* mesh;
    if(type.is(TCAreaItem::BOX)) {
        mesh = generator.generateBox(size);
    } else if(type.is(TCAreaItem::CYLINDER)) {
        mesh = generator.generateCylinder(radius.value(), height.value());
    } else if(type.is(TCAreaItem::SPHERE)) {
        mesh = generator.generateSphere(radius.value());
    }
    SgShape* shape = dynamic_cast<SgShape*>(scene->child(0));
    shape->setMesh(mesh);
    shape->notifyUpdate();
    return true;
}


bool TCAreaItemImpl::onAreaSizePropertyChanged(const string& value)
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


bool TCAreaItemImpl::onAreaRadiusPropertyChanged(const string& value)
{
    double radius = stod(value);
    MeshGenerator generator;

    if(radius >= 0) {
        this->radius = radius;
        SgMesh* mesh;
        if(type.is(TCAreaItem::CYLINDER)) {
            mesh = generator.generateCylinder(radius, height.value());
        } else if(type.is(TCAreaItem::SPHERE)) {
            mesh = generator.generateSphere(radius);
        }
        SgShape* shape = dynamic_cast<SgShape*>(scene->child(0));
        shape->setMesh(mesh);
        shape->notifyUpdate();
        return true;
    }
    return false;
}


bool TCAreaItemImpl::onAreaHeightPropertyChanged(const string& value)
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


bool TCAreaItemImpl::onDiffuseColorPropertyChanged(const string& value)
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


bool TCAreaItemImpl::onEmissiveColorPropertyChanged(const string& value)
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


bool TCAreaItemImpl::onSpecularColorPropertyChanged(const string& value)
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


bool TCAreaItemImpl::onShininessPropertyChanged(const string& value)
{
    double shininess = stod(value);
    if(shininess >= 0) {
        this->shininess = shininess;
        SgShape* shape = dynamic_cast<SgShape*>(scene->child(0));
        SgMaterial* material = shape->material();
        material->setShininess(shininess);
        material->notifyUpdate();
        return true;
    }
    return false;
}


bool TCAreaItemImpl::onTransparencyPropertyChanged(const string& value)
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


void TCAreaItemImpl::generateShape()
{
    SgShape* shape = new SgShape();
    SgMaterial* material = new SgMaterial();
    shape->setMaterial(material);
    scene->addChild(shape, true);
}


Item* TCAreaItem::doDuplicate() const
{
    return new TCAreaItem(*this);
}


void TCAreaItem::doPutProperties(PutPropertyFunction& putProperty)
{
    impl->doPutProperties(putProperty);
}


void TCAreaItemImpl::doPutProperties(PutPropertyFunction& putProperty)
{
    putProperty(_("InboundDelay"), inboundDelay,
                [&](const string& v){ return inboundDelay.setNonNegativeValue(v); });
    putProperty(_("InboundRate"), inboundRate,
                [&](const string& v){ return inboundRate.setNonNegativeValue(v); });
    putProperty(_("InboundLoss"), inboundLoss,
                [&](const string& v){ return inboundLoss.setNonNegativeValue(v); });
    putProperty(_("OutboundDelay"), outboundDelay,
                [&](const string& v){ return outboundDelay.setNonNegativeValue(v); });
    putProperty(_("OutboundRate"), outboundRate,
                [&](const string& v){ return outboundRate.setNonNegativeValue(v); });
    putProperty(_("OutboundLoss"), outboundLoss,
                [&](const string& v){ return outboundLoss.setNonNegativeValue(v); });
    putProperty(_("Source IP"), source, changeProperty(source));
    putProperty(_("Destination IP"), destination, changeProperty(destination));
    putProperty(_("Shape"), type,
                [&](int index){ return onAreaTypePropertyChanged(index); });
    if(type.is(TCAreaItem::BOX)) {
        putProperty(_("Size"), str(size),
                [&](const string& value){ return onAreaSizePropertyChanged(value); });
    } else if(type.is(TCAreaItem::CYLINDER) || type.is(TCAreaItem::SPHERE)) {
        putProperty(_("Radius"), to_string(radius.value()),
                [&](const string& value){ return onAreaRadiusPropertyChanged(value); });
        if(type.is(TCAreaItem::CYLINDER)) {
            putProperty(_("Height"), to_string(height.value()),
                    [&](const string& value){ return onAreaHeightPropertyChanged(value); });
        }
    }
    putProperty(_("Translation"), str(translation),
            [&](const string& value){ return onTranslationPropertyChanged(value); });
    if(type.is(TCAreaItem::CYLINDER)) {
        putProperty(_("Rotation"), str(rotation),
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


bool TCAreaItem::store(Archive& archive)
{
    return impl->store(archive);
}


bool TCAreaItemImpl::store(Archive& archive)
{
    write(archive, "translation", translation);
    write(archive, "rotation", rotation);
    archive.write("type", type.selectedSymbol(), DOUBLE_QUOTED);
    write(archive, "size", size);
    archive.write("radius", radius);
    archive.write("height", height);
    archive.write("inboundDelay", inboundDelay);
    archive.write("inboundRate", inboundRate);
    archive.write("inboundLoss", inboundLoss);
    archive.write("outboundDelay", outboundDelay);
    archive.write("outboundRate", outboundRate);
    archive.write("outboundLoss", outboundLoss);
    archive.write("source", source);
    archive.write("destination", destination);
    write(archive, "diffuseColor", diffuseColor);
    write(archive, "emissiveColor", emissiveColor);
    write(archive, "specularColor", specularColor);
    archive.write("shininess", shininess);
    archive.write("transparency", transparency);
    return true;
}


bool TCAreaItem::restore(const Archive& archive)
{
    return impl->restore(archive);
}


bool TCAreaItemImpl::restore(const Archive& archive)
{
    read(archive, "translation", translation);
    read(archive, "rotation", rotation);
    string symbol;
    if(archive.read("type", symbol)) {
        type.select(symbol);
    }
    read(archive, "size", size);
    radius = archive.get("radius", radius.string());
    height = archive.get("height", height.string());
    inboundDelay = archive.get("inboundDelay", inboundDelay.string());
    inboundRate = archive.get("inboundRate", inboundRate.string());
    inboundLoss = archive.get("inboundLoss", inboundLoss.string());
    outboundDelay = archive.get("outboundDelay", outboundDelay.string());
    outboundRate = archive.get("outboundRate", outboundRate.string());
    outboundLoss = archive.get("outboundLoss", outboundLoss.string());
    archive.read("source", source);
    archive.read("destination", destination);
    read(archive, "diffuseColor", diffuseColor);
    read(archive, "emissiveColor", emissiveColor);
    read(archive, "specularColor", specularColor);
    shininess = archive.get("shininess", shininess.string());
    transparency = archive.get("transparency", transparency.string());
    updateScene();
    return true;
}
