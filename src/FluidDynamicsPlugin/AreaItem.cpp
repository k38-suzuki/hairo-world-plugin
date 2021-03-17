/**
   \file
   \author Kenta Suzuki
*/

#include "AreaItem.h"
#include <cnoid/Archive>
#include <cnoid/EigenArchive>
#include <cnoid/EigenTypes>
#include <cnoid/EigenUtil>
#include <cnoid/FloatingNumberString>
#include <cnoid/ItemManager>
#include <cnoid/MeshGenerator>
#include <cnoid/PutPropertyFunction>
#include <cnoid/Selection>
#include "gettext.h"

using namespace std;
using namespace cnoid;

namespace cnoid {

class AreaItemImpl
{
public:
    AreaItemImpl(AreaItem* self);
    AreaItemImpl(AreaItem* self, const AreaItemImpl& org);

    AreaItem* self;
    Vector3 translation;
    Vector3 rotation;
    Selection type;
    Vector3 size;
    FloatingNumberString radius;
    FloatingNumberString height;
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


AreaItem::AreaItem()
{
    impl = new AreaItemImpl(this);
}


AreaItemImpl::AreaItemImpl(AreaItem* self)
    : self(self)
{
    translation << 0.0, 0.0, 0.0;
    rotation << 0.0, 0.0, 0.0;
    type.setSymbol(AreaItem::BOX, N_("Box"));
    type.setSymbol(AreaItem::CYLINDER, N_("Cylinder"));
    type.setSymbol(AreaItem::SPHERE, N_("Sphere"));
    size << 1.0, 1.0, 1.0;
    radius = 0.5;
    height = 1.0;
    diffuseColor << 0.0, 1.0, 1.0;
    emissiveColor << 0.0, 0.0, 0.0;
    specularColor << 0.0, 0.0, 0.0;
    shininess = 0.0;
    transparency = 0.8;
    scene = new SgPosTransform();
    generateShape();
    updateScene();
}


AreaItem::AreaItem(const AreaItem& org)
    : Item(org),
      impl(new AreaItemImpl(this, *org.impl))
{

}


AreaItemImpl::AreaItemImpl(AreaItem* self, const AreaItemImpl& org)
    : self(self)
{
    translation = org.translation;
    rotation = org.rotation;
    type = org.type;
    size = org.size;
    radius = org.radius;
    height = org.height;
    diffuseColor = org.diffuseColor;
    emissiveColor = org.emissiveColor;
    specularColor = org.specularColor;
    shininess = org.shininess;
    transparency = org.transparency;
    scene = new SgPosTransform();
    generateShape();
    updateScene();
}


AreaItem::~AreaItem()
{
    delete impl;
}


SgNode* AreaItem::getScene()
{
    return impl->scene;
}


void AreaItem::initializeClass(ExtensionManager* ext)
{
    ItemManager& im = ext->itemManager();
    im.registerClass<AreaItem>(N_("AreaItem"));
}


void AreaItem::setTranslation(const Vector3& translation)
{
    impl->translation = translation;
}


Vector3 AreaItem::translation() const
{
    return impl->translation;
}


void AreaItem::setRotation(const Vector3& rotation)
{
    impl->rotation = rotation;
}


Vector3 AreaItem::rotation() const
{
    return impl->rotation;
}


void AreaItem::setType(const int& type)
{
    impl->type.selectIndex(type);
}


int AreaItem::type() const
{
    return impl->type.selectedIndex();
}


void AreaItem::setSize(const Vector3& size)
{
    impl->size = size;
}


Vector3 AreaItem::size() const
{
    return impl->size;
}


void AreaItem::setRadius(const double& radius)
{
    impl->radius = radius;
}


double AreaItem::radius() const
{
    return impl->radius.value();
}


void AreaItem::setHeight(const double& height)
{
    impl->height = height;
}


double AreaItem::height() const
{
    return impl->height.value();
}


void AreaItem::setDiffuseColor(const Vector3& diffuseColor)
{
    impl->diffuseColor = diffuseColor;
}


Vector3 AreaItem::diffuseColor() const
{
    return impl->diffuseColor;
}


void AreaItem::setEmissiveColor(const Vector3& emissiveColor)
{
    impl->emissiveColor = emissiveColor;
}


Vector3 AreaItem::emissiveColor() const
{
    return impl->emissiveColor;
}


void AreaItem::setSpecularColor(const Vector3& specularColor)
{
    impl->specularColor = specularColor;
}


Vector3 AreaItem::specularColor() const
{
    return impl->specularColor;
}


void AreaItem::setShininess(const double& shininess)
{
    impl->shininess = shininess;
}


double AreaItem::shininess() const
{
    return impl->shininess.value();
}


void AreaItem::setTransparency(const double& transparency)
{
    impl->transparency = transparency;
}


double AreaItem::transparency() const
{
    return impl->transparency.value();
}


bool AreaItemImpl::onTranslationPropertyChanged(const string& value)
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


bool AreaItemImpl::onRotationPropertyChanged(const string& value)
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


bool AreaItemImpl::onAreaTypePropertyChanged(const int& index)
{
    MeshGenerator generator;

    type.selectIndex(index);
    SgMesh* mesh;
    if(type.is(AreaItem::BOX)) {
        mesh = generator.generateBox(size);
    } else if(type.is(AreaItem::CYLINDER)) {
        mesh = generator.generateCylinder(radius.value(), height.value());
    } else if(type.is(AreaItem::SPHERE)) {
        mesh = generator.generateSphere(radius.value());
    }
    SgShape* shape = dynamic_cast<SgShape*>(scene->child(0));
    shape->setMesh(mesh);
    shape->notifyUpdate();
    return true;
}


bool AreaItemImpl::onAreaSizePropertyChanged(const string& value)
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


bool AreaItemImpl::onAreaRadiusPropertyChanged(const string& value)
{
    double radius = stod(value);
    MeshGenerator generator;

    if(radius >= 0) {
        this->radius = radius;
        SgMesh* mesh;
        if(type.is(AreaItem::CYLINDER)) {
            mesh = generator.generateCylinder(radius, height.value());
        } else if(type.is(AreaItem::SPHERE)) {
            mesh = generator.generateSphere(radius);
        }
        SgShape* shape = dynamic_cast<SgShape*>(scene->child(0));
        shape->setMesh(mesh);
        shape->notifyUpdate();
        return true;
    }
    return false;
}


bool AreaItemImpl::onAreaHeightPropertyChanged(const string& value)
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


bool AreaItemImpl::onDiffuseColorPropertyChanged(const string& value)
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


bool AreaItemImpl::onEmissiveColorPropertyChanged(const string& value)
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


bool AreaItemImpl::onSpecularColorPropertyChanged(const string& value)
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


bool AreaItemImpl::onShininessPropertyChanged(const string& value)
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


bool AreaItemImpl::onTransparencyPropertyChanged(const string& value)
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


void AreaItemImpl::generateShape()
{
    SgShape* shape = new SgShape();
    SgMaterial* material = new SgMaterial();
    shape->setMaterial(material);
    scene->addChild(shape, true);
}


void AreaItem::updateScene()
{
    impl->updateScene();
}


void AreaItemImpl::updateScene()
{
    MeshGenerator generator;

    scene->setTranslation(translation);
    if(type.is(AreaItem::CYLINDER)) {
        scene->setRotation(rotFromRpy(rotation * TO_RADIAN));
    }
    SgMesh* mesh;
    if(type.is(AreaItem::BOX)) {
        mesh = generator.generateBox(size);
    } else if(type.is(AreaItem::CYLINDER)) {
        mesh = generator.generateCylinder(radius.value(), height.value());
    } else if(type.is(AreaItem::SPHERE)) {
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


Item* AreaItem::doDuplicate() const
{
    return new AreaItem(*this);
}


void AreaItem::doPutProperties(PutPropertyFunction& putProperty)
{
    impl->doPutProperties(putProperty);
}


void AreaItemImpl::doPutProperties(PutPropertyFunction& putProperty)
{
    putProperty(_("Shape"), type,
                [&](int index){ return onAreaTypePropertyChanged(index); });
    if(type.is(AreaItem::BOX)) {
        putProperty(_("Size"), str(size),
                [&](const string& value){ return onAreaSizePropertyChanged(value); });
    } else if(type.is(AreaItem::CYLINDER) || type.is(AreaItem::SPHERE)) {
        putProperty(_("Radius"), to_string(radius.value()),
                [&](const string& value){ return onAreaRadiusPropertyChanged(value); });
        if(type.is(AreaItem::CYLINDER)) {
            putProperty(_("Height"), to_string(height.value()),
                    [&](const string& value){ return onAreaHeightPropertyChanged(value); });
        }
    }
    putProperty(_("Translation"), str(translation),
            [&](const string& value){ return onTranslationPropertyChanged(value); });
    if(type.is(AreaItem::CYLINDER)) {
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


bool AreaItem::store(Archive& archive)
{
    return impl->store(archive);
}


bool AreaItemImpl::store(Archive& archive)
{
    write(archive, "translation", translation);
    write(archive, "rotation", rotation);
    archive.write("type", type.selectedIndex());
    write(archive, "size", size);
    archive.write("radius", radius);
    archive.write("height", height);
    write(archive, "diffuseColor", diffuseColor);
    write(archive, "emissiveColor", emissiveColor);
    write(archive, "specularColor", specularColor);
    archive.write("shininess", shininess);
    archive.write("transparency", transparency);
    return true;
}


bool AreaItem::restore(const Archive &archive)
{
    return impl->restore(archive);
}


bool AreaItemImpl::restore(const Archive& archive)
{
    read(archive, "translation", translation);
    read(archive, "rotation", rotation);
    int t = 0;
    archive.read("type", t);
    type.selectIndex(t);
    read(archive, "size", size);
    radius = archive.get("radius", radius.string());
    height = archive.get("height", height.string());
    read(archive, "diffuseColor", diffuseColor);
    read(archive, "emissiveColor", emissiveColor);
    read(archive, "specularColor", specularColor);
    shininess = archive.get("shininess", shininess.string());
    transparency = archive.get("transparency", transparency.string());
    updateScene();
    return true;
}
