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
#include "gettext.h"

using namespace cnoid;
using namespace std;

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
    FloatingNumberString shininess;
    FloatingNumberString transparency;
    SgPosTransformPtr scene;
    PositionDraggerPtr positionDragger;

    enum AreaTypeID { BOX, CYLINDER, SPHERE, NUM_AREA };

    void updateScene();
    void onPositionDragged();
    bool onTranslationPropertyChanged(const string& value);
    bool onRotationPropertyChanged(const string& value);
    bool onAreaTypePropertyChanged(const int& index);
    bool onAreaAxesPropertyChanged(const int& index);
    bool onAreaSizePropertyChanged(const string& value);
    bool onAreaRadiusPropertyChanged(const string& value);
    bool onAreaHeightPropertyChanged(const string& value);
    bool onDiffuseColorPropertyChanged(const string& value);
    bool onShininessPropertyChanged(const string& value);
    bool onTransparencyPropertyChanged(const string& value);
    bool isCollided(const Vector3& position);
    void doPutProperties(PutPropertyFunction& putProperty);
    bool store(Archive& archive);
    bool restore(const Archive& archive);
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
    type.setSymbol(BOX, N_("Box"));
    type.setSymbol(CYLINDER, N_("Cylinder"));
    type.setSymbol(SPHERE, N_("Sphere"));
    size << 1.0, 1.0, 1.0;
    radius = 0.5;
    height = 1.0;
    diffuseColor << 0.0, 1.0, 1.0;
    shininess = 0.0;
    transparency = 0.8;
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
    shininess = org.shininess;
    transparency = org.transparency;
}


AreaItem::~AreaItem()
{
    delete impl;
}


void AreaItem::initializeClass(ExtensionManager* ext)
{
    ext->itemManager().registerClass<AreaItem>(N_("AreaItem"));
}


SgNode* AreaItem::getScene()
{
    if(!impl->scene) {
        impl->scene = new SgPosTransform;

        SgGroup* group = new SgGroup;
        SgShape* shape = new SgShape;
        SgMaterial* material = new SgMaterial;
        shape->setMaterial(material);
        impl->positionDragger = new PositionDragger(
                    PositionDragger::TranslationAxes, PositionDragger::WideHandle);
        impl->positionDragger->setDragEnabled(true);
        impl->positionDragger->setOverlayMode(true);
        impl->positionDragger->setPixelSize(48, 2);
        impl->positionDragger->setDisplayMode(PositionDragger::DisplayInEditMode);

        group->addChild(shape);
        group->addChild(impl->positionDragger);
        impl->scene->addChild(group, true);

        impl->positionDragger->sigPositionDragged().connect([&](){ impl->onPositionDragged(); });
    }
    impl->updateScene();
    return impl->scene;
}


void AreaItem::setDiffuseColor(const Vector3& diffuseColor)
{
    impl->diffuseColor = diffuseColor;
}


bool AreaItemImpl::onTranslationPropertyChanged(const string& value)
{
    if(toVector3(value, translation)) {
        updateScene();
        return true;
    }
    return false;
}


bool AreaItemImpl::onRotationPropertyChanged(const string& value)
{
    if(toVector3(value, rotation)) {
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
    if(toVector3(value, size)) {
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
    if(toVector3(value, diffuseColor)) {
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


void AreaItemImpl::updateScene()
{
    MeshGenerator generator;

    scene->setTranslation(translation);
    scene->setRotation(rotFromRpy(radian(rotation)));

    SgGroup* group = dynamic_cast<SgGroup*>(scene->child(0));
    if(group) {
        SgShape* shape = dynamic_cast<SgShape*>(group->child(0));
        if(shape) {
            SgMesh* mesh;
            if(type.is(BOX)) {
                mesh = generator.generateBox(size);
            } else if(type.is(CYLINDER)) {
                mesh = generator.generateCylinder(radius.value(), height.value());
            } else if(type.is(SPHERE)) {
                mesh = generator.generateSphere(radius.value());
            }
            SgMaterial* material = shape->material();
            if(material) {
                float s = 127.0f * std::max(0.0f, std::min((float)shininess.value(), 1.0f)) + 1.0f;
                material->setDiffuseColor(diffuseColor);
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


bool AreaItem::isCollided(const Vector3& position)
{
    return impl->isCollided(position);
}


bool AreaItemImpl::isCollided(const Vector3& position)
{
    bool isCollided = false;

    Vector3 rpy = radian(rotation);
    Matrix3 rot = rotFromRpy(rpy);

    int index = type.selectedIndex();
    if(index == BOX) {
        Vector3 min = translation - size / 2.0;
        Vector3 max = translation + size / 2.0;
        Vector3 rp = position - translation;
        Vector3 np = rot.inverse() * rp + translation;

        if((min[0] <= np[0]) && (np[0] <= max[0])
                && (min[1] <= np[1]) && (np[1] <= max[1])
                && (min[2] <= np[2]) && (np[2] <= max[2])
                ) {
            isCollided = true;
        }
    } else if(index == CYLINDER) {
        Vector3 a = rot * (Vector3(0.0, 1.0, 0.0) * height.value() / 2.0) + translation;
        Vector3 b = rot * (Vector3(0.0, 1.0, 0.0) * height.value() / 2.0 * -1.0) + translation;
        Vector3 c = a - b;
        Vector3 d = position - b;
        double cd = c.dot(d);
        if((0.0 < cd) && (cd < c.dot(c))) {
            double r2 = d.dot(d) - d.dot(c) * d.dot(c) / c.dot(c);
            double rp2 =  radius.value() * radius.value();
            if(r2 < rp2) {
                isCollided = true;
            }
        }
    } else if(index == SPHERE) {
        Vector3 r = translation - position;
        if(r.norm() <= radius.value()) {
            isCollided = true;
        }
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
    if(type.is(BOX)) {
        putProperty(_("Size"), str(size),
                [&](const string& value){ return onAreaSizePropertyChanged(value); });
    } else if(type.is(CYLINDER) || type.is(SPHERE)) {
        putProperty(_("Radius"), to_string(radius.value()),
                [&](const string& value){ return onAreaRadiusPropertyChanged(value); });
        if(type.is(CYLINDER)) {
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
    write(archive, "diffuse_color", diffuseColor);
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
    type.selectIndex(archive.get("type", 0));
    read(archive, "size", size);
    radius = archive.get("radius", radius.string());
    height = archive.get("height", height.string());
    read(archive, "diffuse_color", diffuseColor);
    shininess = archive.get("shininess", shininess.string());
    transparency = archive.get("transparency", transparency.string());
    return true;
}
