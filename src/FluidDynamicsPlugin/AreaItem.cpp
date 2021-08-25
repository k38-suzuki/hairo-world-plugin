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
#include <cnoid/PositionDragger>
#include <cnoid/PutPropertyFunction>
#include <cnoid/Selection>
#include <cnoid/WorldItem>
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
    PositionDraggerPtr positionDragger;

    enum AxesType { X, Y, Z, NUM_AXIS };

    void doPutProperties(PutPropertyFunction& putProperty);
    bool store(Archive& archive);
    bool restore(const Archive& archive);
    void generateShape();
    void updateScene();
    bool isCollided(const Link* link);
    void onPositionDragged();
    bool onTranslationPropertyChanged(const string& value);
    bool onRotationPropertyChanged(const string& value);
    bool onAreaTypePropertyChanged(const int& index);
    bool onAreaAxesPropertyChanged(const int& index);
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
    positionDragger = nullptr;
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
    positionDragger = org.positionDragger;
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
        updateScene();
        return true;
    }
    return false;
}


bool AreaItemImpl::onRotationPropertyChanged(const string& value)
{
    Vector3 rotation;
    if(toVector3(value, rotation)) {
        this->rotation = rotation;
        updateScene();
        return true;
    }
    return false;
}


bool AreaItemImpl::onAreaTypePropertyChanged(const int& index)
{
    type.selectIndex(index);
    updateScene();
    return true;
}


bool AreaItemImpl::onAreaSizePropertyChanged(const string& value)
{
    Vector3 size;
    if(toVector3(value, size)) {
        this->size = size;
        updateScene();
        return true;
    }
    return false;
}


bool AreaItemImpl::onAreaRadiusPropertyChanged(const string& value)
{
    double radius = stod(value);
    if(radius >= 0) {
        this->radius = radius;
        updateScene();
        return true;
    }
    return false;
}


bool AreaItemImpl::onAreaHeightPropertyChanged(const string& value)
{
    double height = stod(value);
    if(height >= 0) {
        this->height = height;
        updateScene();
        return true;
    }
    return false;
}


bool AreaItemImpl::onDiffuseColorPropertyChanged(const string& value)
{
    Vector3 diffuseColor;
    if(toVector3(value, diffuseColor)) {
        this->diffuseColor = diffuseColor;
        updateScene();
        return true;
    }
    return false;
}


bool AreaItemImpl::onEmissiveColorPropertyChanged(const string& value)
{
    Vector3 emissiveColor;
    if(toVector3(value, emissiveColor)) {
        this->emissiveColor = emissiveColor;
        updateScene();
        return true;
    }
    return false;
}


bool AreaItemImpl::onSpecularColorPropertyChanged(const string& value)
{
    Vector3 specularColor;
    if(toVector3(value, specularColor)) {
        this->specularColor = specularColor;
        updateScene();
        return true;
    }
    return false;
}


bool AreaItemImpl::onShininessPropertyChanged(const string& value)
{
    float shininess = stof(value);
    if(shininess >= 0) {
        this->shininess = shininess;
        updateScene();
        return true;
    }
    return false;
}


bool AreaItemImpl::onTransparencyPropertyChanged(const string& value)
{
    double transparency = stod(value);
    if(transparency >= 0) {
        this->transparency = transparency;
        updateScene();
        return true;
    }
    return false;
}


void AreaItemImpl::onPositionDragged()
{
    translation = positionDragger->globalDraggingPosition().translation();
    updateScene();
}


void AreaItemImpl::generateShape()
{
    SgGroup* group = new SgGroup();
    SgShape* shape = new SgShape();
    SgMaterial* material = new SgMaterial();
    shape->setMaterial(material);
    positionDragger = new PositionDragger(
                PositionDragger::TranslationAxes, PositionDragger::WideHandle);
    positionDragger->setDragEnabled(true);
    positionDragger->setOverlayMode(true);
    positionDragger->setPixelSize(48, 2);
    positionDragger->setDisplayMode(PositionDragger::DisplayInEditMode);

    group->addChild(shape);
    group->addChild(positionDragger);
    scene->addChild(group, true);

    positionDragger->sigPositionDragged().connect([&](){ onPositionDragged(); });
}


void AreaItem::updateScene()
{
    impl->updateScene();
}


void AreaItemImpl::updateScene()
{
    MeshGenerator generator;

    scene->setTranslation(translation);
    scene->setRotation(rotFromRpy(rotation * TO_RADIAN));

    SgGroup* group = dynamic_cast<SgGroup*>(scene->child(0));
    if(group) {
        SgShape* shape = dynamic_cast<SgShape*>(group->child(0));
        if(shape) {
            SgMesh* mesh;
            if(type.is(AreaItem::BOX)) {
                mesh = generator.generateBox(size);
            } else if(type.is(AreaItem::CYLINDER)) {
                mesh = generator.generateCylinder(radius.value(), height.value());
            } else if(type.is(AreaItem::SPHERE)) {
                mesh = generator.generateSphere(radius.value());
            }
            SgMaterial* material = shape->material();
            if(material) {
                float s = 127.0f * std::max(0.0f, std::min((float)shininess.value(), 1.0f)) + 1.0f;
                material->setDiffuseColor(diffuseColor);
                material->setEmissiveColor(emissiveColor);
                material->setSpecularColor(specularColor);
                material->setSpecularExponent(s);
                material->setTransparency(transparency.value());
            }
            shape->setMesh(mesh);
            shape->setMaterial(material);
            shape->notifyUpdate();
            positionDragger->adjustSize(shape->boundingBox());
        }
    }
}


bool AreaItem::isCollided(const Link* link)
{
    return impl->isCollided(link);
}


bool AreaItemImpl::isCollided(const Link* link)
{
    bool isCollided = false;

    AreaItem* item = self;
    Vector3 p = link->T().translation();
    Vector3 translation = item->translation();
    Vector3 rpy = item->rotation() * TO_RADIAN;
    Matrix3 rot = rotFromRpy(rpy);

    if(item->type() == AreaItem::BOX) {
        Vector3 size = item->size();
        Vector3 min = translation - size / 2.0;
        Vector3 max = translation + size / 2.0;
        Vector3 rp = p - translation;
        Vector3 np = rot.inverse() * rp + translation;

        if((min[0] <= np[0]) && (np[0] <= max[0])
                && (min[1] <= np[1]) && (np[1] <= max[1])
                && (min[2] <= np[2]) && (np[2] <= max[2])
                ) {
            isCollided = true;
        }
    } else if(item->type() == AreaItem::CYLINDER) {
        Vector3 a = rot * (Vector3(0.0, 1.0, 0.0) * item->height() / 2.0) + translation;
        Vector3 b = rot * (Vector3(0.0, 1.0, 0.0) * item->height() / 2.0 * -1.0) + translation;
        Vector3 c = a - b;
        Vector3 d = p - b;
        double cd = c.dot(d);
        if((0.0 < cd) && (cd < c.dot(c))) {
            double r2 = d.dot(d) - d.dot(c) * d.dot(c) / c.dot(c);
            double rp2 =  item->radius() * item->radius();
            if(r2 < rp2) {
                isCollided = true;
            }
        }
    } else if(item->type() == AreaItem::SPHERE) {
        Vector3 r = translation - p;
        if(r.norm() <= item->radius()) {
            isCollided = true;
        }
    }
    WorldItem* worldItem = item->findOwnerItem<WorldItem>();
    if(!worldItem) {
        isCollided = false;
    }

    return isCollided;
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
    putProperty(_("RPY"), str(rotation),
            [&](const string& value){ return onRotationPropertyChanged(value); });
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
